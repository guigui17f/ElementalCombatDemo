// Copyright 2025 guigui17f. All Rights Reserved.

#include "CombatProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ACombatProjectile::ACombatProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建碰撞组件
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(10.0f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;

	// 创建投掷物运动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = ProjectileConfig.InitialSpeed;
	ProjectileMovement->MaxSpeed = ProjectileConfig.MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = ProjectileConfig.bShouldBounce;
	ProjectileMovement->ProjectileGravityScale = ProjectileConfig.GravityScale;
	ProjectileMovement->Bounciness = ProjectileConfig.BounceDamping;

	// 创建粒子组件（可选）
	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	ParticleComp->SetupAttachment(RootComponent);

	// 设置生命周期
	InitialLifeSpan = ProjectileConfig.LifeSpan;

	// 绑定碰撞事件
	CollisionComp->OnComponentHit.AddDynamic(this, &ACombatProjectile::OnHit);
}

void ACombatProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 初始化伤害值
	CurrentDamage = ProjectileConfig.BaseDamage;

	// 应用配置到运动组件
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = ProjectileConfig.InitialSpeed;
		ProjectileMovement->MaxSpeed = ProjectileConfig.MaxSpeed;
		ProjectileMovement->bShouldBounce = ProjectileConfig.bShouldBounce;
		ProjectileMovement->ProjectileGravityScale = ProjectileConfig.GravityScale;
		ProjectileMovement->Bounciness = ProjectileConfig.BounceDamping;
	}

	// 设置生命周期
	SetLifeSpan(ProjectileConfig.LifeSpan);

	// 播放发射音效
	if (LaunchSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation());
	}

	// 生成轨迹特效
	if (TrailEffect)
	{
		UNiagaraComponent* TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailEffect,
			CollisionComp,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true,
			true
		);
		
		if (TrailComponent)
		{
			TrailComponent->SetVariableFloat(FName("User.LifeTime"), ProjectileConfig.LifeSpan);
		}
	}

	// 忽略发射者的碰撞
	if (GetInstigator())
	{
		CollisionComp->MoveIgnoreActors.Add(GetInstigator());
	}
	if (GetOwner())
	{
		CollisionComp->MoveIgnoreActors.Add(GetOwner());
	}
}

void ACombatProjectile::SetProjectileProperties(float InSpeedMultiplier, float InDamageMultiplier)
{
	SpeedMultiplier = FMath::Max(InSpeedMultiplier, 0.1f);
	DamageMultiplier = FMath::Max(InDamageMultiplier, 0.1f);

	// 更新伤害值
	CurrentDamage = ProjectileConfig.BaseDamage * DamageMultiplier;

	// 更新速度
	if (ProjectileMovement)
	{
		float NewSpeed = ProjectileConfig.InitialSpeed * SpeedMultiplier;
		NewSpeed = FMath::Min(NewSpeed, ProjectileConfig.MaxSpeed);
		
		ProjectileMovement->InitialSpeed = NewSpeed;
		ProjectileMovement->MaxSpeed = FMath::Max(NewSpeed, ProjectileConfig.MaxSpeed);
		
		// 如果已经在运动，更新当前速度
		if (!ProjectileMovement->Velocity.IsZero())
		{
			ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * NewSpeed;
		}
	}
}

void ACombatProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 忽略自己和发射者
	if (OtherActor && OtherActor != this && OtherActor != GetOwner() && OtherActor != GetInstigator())
	{
		// 应用伤害
		ApplyDamageToTarget(OtherActor, Hit);

		// 播放碰撞效果
		PlayImpactEffects(Hit.Location, Hit.Normal);
		
		// 生成撞击特效
		if (ImpactEffect)
		{
			FRotator ImpactRotation = Hit.Normal.Rotation();
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				ImpactEffect,
				Hit.Location,
				ImpactRotation,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true
			);
		}

		// 播放碰撞音效
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.Location);
		}

		// 销毁投掷物
		Destroy();
	}
}

void ACombatProjectile::ApplyDamageToTarget(AActor* Target, const FHitResult& Hit)
{
	if (!Target) return;

	// 尝试使用接口应用伤害
	if (ICombatDamageable* DamageableTarget = Cast<ICombatDamageable>(Target))
	{
		// 计算击退方向和力度
		FVector ImpactDirection = ProjectileMovement->Velocity.GetSafeNormal();
		float ImpactForce = 250.0f * DamageMultiplier; // 基础击退力 * 伤害倍率
		FVector DamageImpulse = ImpactDirection * ImpactForce;

		// 通过接口应用伤害
		DamageableTarget->ApplyDamage(CurrentDamage, this, Hit.Location, DamageImpulse);
	}
	else
	{
		// 使用标准伤害系统
		FDamageEvent DamageEvent;
		Target->TakeDamage(CurrentDamage, DamageEvent, nullptr, this);
	}
}

void ACombatProjectile::SetProjectileConfig(const FProjectileConfig& NewConfig)
{
	ProjectileConfig = NewConfig;
	
	// 更新运行时属性
	CurrentDamage = ProjectileConfig.BaseDamage * DamageMultiplier;
	
	// 更新运动组件属性
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = ProjectileConfig.InitialSpeed * SpeedMultiplier;
		ProjectileMovement->MaxSpeed = ProjectileConfig.MaxSpeed;
		ProjectileMovement->ProjectileGravityScale = ProjectileConfig.GravityScale;
		ProjectileMovement->bShouldBounce = ProjectileConfig.bShouldBounce;
		ProjectileMovement->Bounciness = ProjectileConfig.BounceDamping;
	}
	
	// 更新生命周期
	SetLifeSpan(ProjectileConfig.LifeSpan);
}

void ACombatProjectile::SetProjectileSpeed(float NewSpeed)
{
	ProjectileConfig.InitialSpeed = NewSpeed;
	
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = NewSpeed * SpeedMultiplier;
		// 如果投掷物已经在飞行中，更新当前速度
		if (!ProjectileMovement->Velocity.IsZero())
		{
			FVector Direction = ProjectileMovement->Velocity.GetSafeNormal();
			ProjectileMovement->Velocity = Direction * (NewSpeed * SpeedMultiplier);
		}
	}
}

void ACombatProjectile::SetProjectileDamage(float NewDamage)
{
	ProjectileConfig.BaseDamage = NewDamage;
	CurrentDamage = NewDamage * DamageMultiplier;
}

float ACombatProjectile::GetCurrentSpeed() const
{
	if (ProjectileMovement)
	{
		return ProjectileMovement->Velocity.Size();
	}
	return 0.0f;
}

void ACombatProjectile::InitializeLaunchWithAngle(const FVector& ForwardDirection, float LaunchAngleDegrees)
{
	// 获取水平前向方向
	FVector HorizontalForward = FVector(ForwardDirection.X, ForwardDirection.Y, 0.0f).GetSafeNormal();

	// 如果没有有效的水平方向，使用Actor前向
	if (HorizontalForward.IsNearlyZero())
	{
		HorizontalForward = GetActorForwardVector();
		HorizontalForward.Z = 0.0f;
		HorizontalForward.Normalize();
	}

	// 根据角度计算发射速度向量
	float AngleRad = FMath::DegreesToRadians(LaunchAngleDegrees);
	float Speed = ProjectileConfig.InitialSpeed * SpeedMultiplier;
	float HorizontalSpeed = Speed * FMath::Cos(AngleRad);
	float VerticalSpeed = Speed * FMath::Sin(AngleRad);

	// 构建最终速度向量
	FVector LaunchVelocity = HorizontalForward * HorizontalSpeed;
	LaunchVelocity.Z = VerticalSpeed;

	// 设置投掷物速度
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = LaunchVelocity;
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = ProjectileConfig.MaxSpeed * SpeedMultiplier;

		// 确保旋转跟随速度（虽然构造函数已设置，但再次确认）
		ProjectileMovement->bRotationFollowsVelocity = true;
	}

	// 立即设置投掷物朝向为飞行方向
	SetActorRotation(LaunchVelocity.Rotation());

	UE_LOG(LogTemp, Log, TEXT("投掷物发射: 角度=%.1f度, 速度=%.1f, 朝向=(Pitch:%.1f, Yaw:%.1f)"),
		LaunchAngleDegrees, Speed, LaunchVelocity.Rotation().Pitch, LaunchVelocity.Rotation().Yaw);
}

float ACombatProjectile::CalculateAngleForDistance(float TargetDistance, float HeightDifference) const
{
	// 使用投掷物自身的速度配置
	float BaseSpeed = ProjectileConfig.InitialSpeed * SpeedMultiplier;

	// 获取世界重力并应用投掷物的重力缩放
	float Gravity = FMath::Abs(GetWorld()->GetGravityZ()) * ProjectileConfig.GravityScale;

	// 防止除零和无效输入
	if (TargetDistance <= 0.0f || BaseSpeed <= 0.0f || Gravity <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("CalculateAngleForDistance: 无效参数 - 距离:%.1f, 速度:%.1f, 重力:%.1f"),
			TargetDistance, BaseSpeed, Gravity);
		return 15.0f; // 返回默认角度
	}

	// 考虑高度差的弹道计算
	// 使用完整的斜抛运动方程

	// 第一步：尝试15度固定角度
	const float PreferredAngle = 15.0f;
	const float MaxAngle = 30.0f;

	// 计算15度角下能到达的水平距离（考虑高度差）
	// 完整公式: x = (v²/g) * sin(2θ) * (1 + sqrt(1 + 2gh/(v²sin²θ)))
	float AngleRad15 = FMath::DegreesToRadians(PreferredAngle);
	float SinAngle15 = FMath::Sin(AngleRad15);
	float Sin2Angle15 = FMath::Sin(2.0f * AngleRad15);

	// 计算高度修正因子
	float HeightFactor15 = 1.0f;
	if (FMath::Abs(HeightDifference) > 0.1f) // 避免微小高度差的计算误差
	{
		float HeightTerm = 2.0f * Gravity * HeightDifference / (BaseSpeed * BaseSpeed * SinAngle15 * SinAngle15);
		if (HeightTerm > -1.0f) // 确保平方根内容为正
		{
			HeightFactor15 = 1.0f + FMath::Sqrt(1.0f + HeightTerm);
		}
	}

	float Range15 = (BaseSpeed * BaseSpeed / Gravity) * Sin2Angle15 * HeightFactor15;

	if (Range15 >= TargetDistance)
	{
		// 15度角落点过远，反推需要的速度
		// 反推时需要考虑高度差，使用迭代法求解
		float RequiredSpeed = BaseSpeed;
		for (int32 i = 0; i < 5; ++i) // 最多5次迭代
		{
			float TestHeightFactor = 1.0f;
			if (FMath::Abs(HeightDifference) > 0.1f)
			{
				float TestHeightTerm = 2.0f * Gravity * HeightDifference / (RequiredSpeed * RequiredSpeed * SinAngle15 * SinAngle15);
				if (TestHeightTerm > -1.0f)
				{
					TestHeightFactor = 1.0f + FMath::Sqrt(1.0f + TestHeightTerm);
				}
			}
			float TestRange = (RequiredSpeed * RequiredSpeed / Gravity) * Sin2Angle15 * TestHeightFactor;

			if (FMath::Abs(TestRange - TargetDistance) < 1.0f) break; // 精度足够

			RequiredSpeed *= FMath::Sqrt(TargetDistance / TestRange);
		}

		float RequiredSpeedMultiplier = RequiredSpeed / (ProjectileConfig.InitialSpeed * SpeedMultiplier);
		const_cast<ACombatProjectile*>(this)->SpeedMultiplier *= RequiredSpeedMultiplier;

		UE_LOG(LogTemp, Log, TEXT("AI投掷物15度角速度调整: 新倍率=%.2f, 目标距离=%.1f, 高度差=%.1f"),
			SpeedMultiplier, TargetDistance, HeightDifference);

		return PreferredAngle;
	}

	// 第二步：15度角落点过近，尝试增加角度
	// 对于有高度差的情况，使用数值方法寻找合适的角度
	float BestAngle = PreferredAngle;
	float BestRangeDiff = FMath::Abs(Range15 - TargetDistance);

	for (float TestAngle = PreferredAngle + 1.0f; TestAngle <= MaxAngle; TestAngle += 1.0f)
	{
		float TestAngleRad = FMath::DegreesToRadians(TestAngle);
		float TestSinAngle = FMath::Sin(TestAngleRad);
		float TestSin2Angle = FMath::Sin(2.0f * TestAngleRad);

		float TestHeightFactor = 1.0f;
		if (FMath::Abs(HeightDifference) > 0.1f)
		{
			float TestHeightTerm = 2.0f * Gravity * HeightDifference / (BaseSpeed * BaseSpeed * TestSinAngle * TestSinAngle);
			if (TestHeightTerm > -1.0f)
			{
				TestHeightFactor = 1.0f + FMath::Sqrt(1.0f + TestHeightTerm);
			}
		}

		float TestRange = (BaseSpeed * BaseSpeed / Gravity) * TestSin2Angle * TestHeightFactor;
		float RangeDiff = FMath::Abs(TestRange - TargetDistance);

		if (RangeDiff < BestRangeDiff)
		{
			BestAngle = TestAngle;
			BestRangeDiff = RangeDiff;
		}

		if (TestRange >= TargetDistance)
		{
			UE_LOG(LogTemp, Log, TEXT("AI投掷物角度调整: 使用角度=%.1f°, 目标距离=%.1f, 高度差=%.1f"),
				BestAngle, TargetDistance, HeightDifference);
			return BestAngle;
		}
	}

	// 第三步：即使30度角也不够，增加速度
	float AngleRad30 = FMath::DegreesToRadians(MaxAngle);
	float SinAngle30 = FMath::Sin(AngleRad30);
	float Sin2Angle30 = FMath::Sin(2.0f * AngleRad30);

	// 反推所需速度（考虑高度差）
	float RequiredSpeed = BaseSpeed;
	for (int32 i = 0; i < 5; ++i)
	{
		float TestHeightFactor = 1.0f;
		if (FMath::Abs(HeightDifference) > 0.1f)
		{
			float TestHeightTerm = 2.0f * Gravity * HeightDifference / (RequiredSpeed * RequiredSpeed * SinAngle30 * SinAngle30);
			if (TestHeightTerm > -1.0f)
			{
				TestHeightFactor = 1.0f + FMath::Sqrt(1.0f + TestHeightTerm);
			}
		}
		float TestRange = (RequiredSpeed * RequiredSpeed / Gravity) * Sin2Angle30 * TestHeightFactor;

		if (FMath::Abs(TestRange - TargetDistance) < 1.0f) break;

		RequiredSpeed *= FMath::Sqrt(TargetDistance / TestRange);
	}

	float MaxSpeed = ProjectileConfig.MaxSpeed;

	if (RequiredSpeed <= MaxSpeed)
	{
		float RequiredSpeedMultiplier = RequiredSpeed / (ProjectileConfig.InitialSpeed * SpeedMultiplier);
		const_cast<ACombatProjectile*>(this)->SpeedMultiplier *= RequiredSpeedMultiplier;

		UE_LOG(LogTemp, Log, TEXT("AI投掷物30度角速度调整: 新倍率=%.2f, 目标距离=%.1f, 高度差=%.1f"),
			SpeedMultiplier, TargetDistance, HeightDifference);

		return MaxAngle;
	}
	else
	{
		// 使用最大速度
		float MaxSpeedMultiplier = MaxSpeed / (ProjectileConfig.InitialSpeed * SpeedMultiplier);
		const_cast<ACombatProjectile*>(this)->SpeedMultiplier *= MaxSpeedMultiplier;

		// 计算实际落点距离
		float ActualHeightFactor = 1.0f;
		if (FMath::Abs(HeightDifference) > 0.1f)
		{
			float ActualHeightTerm = 2.0f * Gravity * HeightDifference / (MaxSpeed * MaxSpeed * SinAngle30 * SinAngle30);
			if (ActualHeightTerm > -1.0f)
			{
				ActualHeightFactor = 1.0f + FMath::Sqrt(1.0f + ActualHeightTerm);
			}
		}
		float ActualRange = (MaxSpeed * MaxSpeed / Gravity) * Sin2Angle30 * ActualHeightFactor;

		UE_LOG(LogTemp, Warning, TEXT("AI投掷物无法到达目标距离%.1f，使用最大速度，实际落点=%.1f, 高度差=%.1f"),
			TargetDistance, ActualRange, HeightDifference);

		return MaxAngle;
	}
}