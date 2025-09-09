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