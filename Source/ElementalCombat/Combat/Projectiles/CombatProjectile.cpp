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
#include "Combat/Elemental/ElementalComponent.h"
#include "Combat/Elemental/ElementalTypes.h"

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

	// 获取发射者的元素组件（如果有）
	FElementalEffectData AttackerEffectData;
	bool bHasElementalData = false;

	if (AActor* MyOwner = GetOwner())
	{
		if (UElementalComponent* OwnerElemental = MyOwner->FindComponentByClass<UElementalComponent>())
		{
			EElementalType OwnerElement = OwnerElemental->GetCurrentElement();
			bHasElementalData = OwnerElemental->GetElementEffectData(OwnerElement, AttackerEffectData);

			if (bHasElementalData)
			{
				UE_LOG(LogTemp, Log, TEXT("CombatProjectile: Using elemental data from %s (Element: %d)"),
					*MyOwner->GetName(), (int32)OwnerElement);
			}
		}
	}

	float FinalDamage = CurrentDamage;

	// 如果目标有元素组件，通过它处理元素效果
	if (UElementalComponent* TargetElemental = Target->FindComponentByClass<UElementalComponent>())
	{
		if (bHasElementalData)
		{
			// 处理元素伤害（包括相克、倍率、减伤等）
			FinalDamage = TargetElemental->ProcessElementalDamage(
				CurrentDamage, AttackerEffectData, GetOwner());

			UE_LOG(LogTemp, Log, TEXT("CombatProjectile: Processed elemental damage on %s (%.1f -> %.1f)"),
				*Target->GetName(), CurrentDamage, FinalDamage);

			// 应用元素效果（减速、DOT、吸血等）
			TargetElemental->ApplyElementalEffects(
				AttackerEffectData, GetOwner(), FinalDamage);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("CombatProjectile: No elemental data available for projectile from %s"),
				GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("CombatProjectile: Target %s has no ElementalComponent, using standard damage"),
			*Target->GetName());
	}

	// 应用最终伤害
	if (ICombatDamageable* DamageableTarget = Cast<ICombatDamageable>(Target))
	{
		// 计算击退方向和力度
		FVector ImpactDirection = ProjectileMovement->Velocity.GetSafeNormal();
		float ImpactForce = 250.0f * DamageMultiplier; // 基础击退力 * 伤害倍率
		FVector DamageImpulse = ImpactDirection * ImpactForce;

		// 通过接口应用伤害
		DamageableTarget->ApplyDamage(FinalDamage, this, Hit.Location, DamageImpulse);
	}
	else
	{
		// 使用标准伤害系统
		FDamageEvent DamageEvent;
		Target->TakeDamage(FinalDamage, DamageEvent, nullptr, this);
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
		return 15.0f;
	}

	const float PreferredAngle = 15.0f;
	const float MaxAngle = 30.0f;
	const float MaxSpeed = ProjectileConfig.MaxSpeed;

	// 精确计算有高度差的射程（给定速度和角度）
	auto CalculateRangeWithHeight = [&](float Speed, float AngleDegrees) -> float
	{
		float AngleRad = FMath::DegreesToRadians(AngleDegrees);
		float SinAngle = FMath::Sin(AngleRad);
		float Sin2Angle = FMath::Sin(2.0f * AngleRad);

		// 精确公式: R = (v²/g) * sin(2θ) * [1 + √(1 + 2gh/(v²sin²θ))] / 2
		float BaseRange = (Speed * Speed / Gravity) * Sin2Angle;

		if (FMath::Abs(HeightDifference) < 0.1f)
		{
			// 无高度差，使用基础抛物线公式
			return BaseRange;
		}

		// 使用负的HeightDifference，因为物理公式中h表示落点相对于发射点的高度
		// HeightDifference > 0表示目标更高，在公式中需要为负值
		float HeightTerm = -2.0f * Gravity * HeightDifference / (Speed * Speed * SinAngle * SinAngle);

		// 确保平方根内容非负
		if (1.0f + HeightTerm < 0.0f)
		{
			return 0.0f; // 无法到达目标
		}

		float HeightFactor = (1.0f + FMath::Sqrt(1.0f + HeightTerm)) / 2.0f;
		return BaseRange * HeightFactor;
	};

	// 第一步：尝试15度固定角度，使用数值方法求解精确速度
	// 由于精确公式复杂，使用二分查找求解所需速度
	auto FindRequiredSpeed = [&](float AngleDegrees) -> float
	{
		float MinSpeed = 100.0f;
		float MaxSearchSpeed = ProjectileConfig.MaxSpeed * 2.0f;
		const float Tolerance = 1.0f; // 1单位容差
		const int MaxIterations = 10;

		for (int32 i = 0; i < MaxIterations; ++i)
		{
			float TestSpeed = (MinSpeed + MaxSearchSpeed) / 2.0f;
			float TestRange = CalculateRangeWithHeight(TestSpeed, AngleDegrees);

			if (FMath::Abs(TestRange - TargetDistance) < Tolerance)
			{
				return TestSpeed; // 找到合适的速度
			}

			if (TestRange > TargetDistance)
			{
				MaxSearchSpeed = TestSpeed; // 速度过高，降低上限
			}
			else
			{
				MinSpeed = TestSpeed; // 速度过低，提高下限
			}
		}

		return (MinSpeed + MaxSearchSpeed) / 2.0f; // 返回最接近的值
	};

	float RequiredSpeed15 = FindRequiredSpeed(PreferredAngle);

	if (RequiredSpeed15 <= BaseSpeed)
	{
		// 15度角可以达到目标，调整速度
		float RequiredSpeedMultiplier = RequiredSpeed15 / ProjectileConfig.InitialSpeed;
		const_cast<ACombatProjectile*>(this)->SpeedMultiplier = RequiredSpeedMultiplier;

		UE_LOG(LogTemp, Log, TEXT("AI投掷物15度角精确速度调整: 新倍率=%.2f, 目标距离=%.1f, 高度差=%.1f"),
			SpeedMultiplier, TargetDistance, HeightDifference);

		return PreferredAngle;
	}

	// 第二步：15度角需要的速度太高，使用固定速度计算最优角度
	// 使用二分查找在15-30度范围内寻找最优角度
	auto FindOptimalAngle = [&]() -> float
	{
		float MinAngle = PreferredAngle;
		float MaxSearchAngle = MaxAngle;
		float BestAngle = PreferredAngle;
		float BestRangeDiff = FLT_MAX;
		const float AngleTolerance = 0.5f; // 0.5度容差
		const int MaxIterations = 10;

		// 先检查边界条件
		float RangeAtMin = CalculateRangeWithHeight(BaseSpeed, MinAngle);
		float RangeAtMax = CalculateRangeWithHeight(BaseSpeed, MaxSearchAngle);

		if (RangeAtMax < TargetDistance)
		{
			// 即使30度也不够，返回30度
			return MaxAngle;
		}

		if (RangeAtMin >= TargetDistance)
		{
			// 15度就足够，返回15度
			return PreferredAngle;
		}

		// 使用二分查找
		for (int32 i = 0; i < MaxIterations; ++i)
		{
			float TestAngle = (MinAngle + MaxSearchAngle) / 2.0f;
			float TestRange = CalculateRangeWithHeight(BaseSpeed, TestAngle);
			float RangeDiff = FMath::Abs(TestRange - TargetDistance);

			if (RangeDiff < BestRangeDiff)
			{
				BestAngle = TestAngle;
				BestRangeDiff = RangeDiff;
			}

			if (RangeDiff < AngleTolerance)
			{
				return TestAngle; // 找到足够精确的角度
			}

			if (TestRange > TargetDistance)
			{
				MaxSearchAngle = TestAngle; // 角度过大，降低上限
			}
			else
			{
				MinAngle = TestAngle; // 角度过小，提高下限
			}
		}

		return BestAngle;
	};

	float OptimalAngleDegrees = FindOptimalAngle();

	// 如果找到了有效角度（不是边界的30度）
	if (OptimalAngleDegrees < MaxAngle)
	{
		UE_LOG(LogTemp, Log, TEXT("AI投掷物精确角度调整: 使用角度=%.1f°, 目标距离=%.1f, 高度差=%.1f"),
			OptimalAngleDegrees, TargetDistance, HeightDifference);

		return OptimalAngleDegrees;
	}

	// 第三步：即使30度角也无法达到，使用30度角并计算所需速度
	float RequiredSpeed30 = FindRequiredSpeed(MaxAngle);

	if (RequiredSpeed30 <= MaxSpeed)
	{
		// 在最大速度范围内，调整速度
		float RequiredSpeedMultiplier = RequiredSpeed30 / ProjectileConfig.InitialSpeed;
		const_cast<ACombatProjectile*>(this)->SpeedMultiplier = RequiredSpeedMultiplier;

		UE_LOG(LogTemp, Log, TEXT("AI投掷物30度角精确速度调整: 新倍率=%.2f, 目标距离=%.1f, 高度差=%.1f"),
			SpeedMultiplier, TargetDistance, HeightDifference);

		return MaxAngle;
	}
	else
	{
		// 使用最大速度，计算实际可达距离
		float MaxSpeedMultiplier = MaxSpeed / ProjectileConfig.InitialSpeed;
		const_cast<ACombatProjectile*>(this)->SpeedMultiplier = MaxSpeedMultiplier;

		float ActualRange = CalculateRangeWithHeight(MaxSpeed, MaxAngle);

		UE_LOG(LogTemp, Log, TEXT("AI投掷物无法到达目标距离%.1f，使用最大速度，精确落点=%.1f, 高度差=%.1f"),
			TargetDistance, ActualRange, HeightDifference);

		return MaxAngle;
	}
}