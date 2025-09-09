// Copyright 2025 guigui17f. All Rights Reserved.

#include "AdvancedCombatCharacter.h"
#include "Projectiles/CombatProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"

AAdvancedCombatCharacter::AAdvancedCombatCharacter()
{
	// 默认配置
	ProjectileSocketName = TEXT("hand_r");
}

void AAdvancedCombatCharacter::DoChargedAttackStart()
{
	// 调用父类实现（播放动画等）
	Super::DoChargedAttackStart();

	// 记录蓄力开始时间
	ChargeStartTime = GetWorld()->GetTimeSeconds();
}

void AAdvancedCombatCharacter::DoChargedAttackEnd()
{
	// 不调用父类的近战攻击实现
	// 改为发射投掷物
	
	// 结束蓄力状态
	bIsChargingAttack = false;
}

void AAdvancedCombatCharacter::LaunchProjectile()
{
	// 检查是否有投掷物类
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectileClass not set for %s"), *GetName());
		return;
	}

	// 计算蓄力时间
	float ChargeTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	
	// 获取发射位置和方向
	FVector SpawnLocation;
	FRotator SpawnRotation;
	GetProjectileLaunchParams(SpawnLocation, SpawnRotation);

	// 设置生成参数
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 生成投掷物
	ACombatProjectile* Projectile = GetWorld()->SpawnActor<ACombatProjectile>(
		ProjectileClass, 
		SpawnLocation, 
		SpawnRotation, 
		SpawnParams
	);

	if (Projectile)
	{
		// 根据蓄力时间调整投掷物属性
		float SpeedMultiplier = 1.0f;
		float DamageMultiplier = 1.0f;

		// 使用曲线计算倍率
		if (ChargeSpeedCurve)
		{
			SpeedMultiplier = ChargeSpeedCurve->GetFloatValue(ChargeTime);
		}
		else
		{
			// 默认的蓄力倍率计算
			SpeedMultiplier = GetChargeMultiplier(ChargeTime);
		}

		if (ChargeDamageCurve)
		{
			DamageMultiplier = ChargeDamageCurve->GetFloatValue(ChargeTime);
		}
		else
		{
			// 默认的蓄力倍率计算
			DamageMultiplier = GetChargeMultiplier(ChargeTime);
		}

		// 应用倍率到投掷物
		Projectile->SetProjectileProperties(SpeedMultiplier, DamageMultiplier);

		// 触发蓝图事件
		OnProjectileLaunched(Projectile);
	}

	// 重置蓄力时间
	ChargeStartTime = 0.0f;
}

float AAdvancedCombatCharacter::GetChargeMultiplier(float ChargeTime) const
{
	// 默认的蓄力倍率计算
	if (ChargeTime < 0.5f)
	{
		return 1.0f;
	}
	else if (ChargeTime < 1.0f)
	{
		return 1.5f;
	}
	else
	{
		return 2.0f;
	}
}

void AAdvancedCombatCharacter::GetProjectileLaunchParams(FVector& OutLocation, FRotator& OutRotation) const
{
	// 获取发射位置（从手部Socket）
	if (GetMesh() && GetMesh()->DoesSocketExist(ProjectileSocketName))
	{
		OutLocation = GetMesh()->GetSocketLocation(ProjectileSocketName);
	}
	else
	{
		// 如果没有Socket，使用角色位置加偏移
		OutLocation = GetActorLocation() + FVector(0, 0, 100.0f);
	}

	// 获取发射方向（使用控制器旋转）
	if (GetController())
	{
		OutRotation = GetControlRotation();
	}
	else
	{
		// 如果没有控制器，使用角色朝向
		OutRotation = GetActorRotation();
	}
}