// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/CombatCharacter.h"
#include "Combat/Projectiles/CombatProjectile.h"
#include "AdvancedCombatCharacter.generated.h"

class UCurveFloat;

/**
 * 高级战斗角色类
 * 继承自CombatCharacter，将蓄力攻击改为发射投掷物
 * 支持远程战斗能力的高级战斗角色
 */
UCLASS()
class ELEMENTALCOMBAT_API AAdvancedCombatCharacter : public ACombatCharacter
{
	GENERATED_BODY()

public:
	AAdvancedCombatCharacter();

protected:
	// 投掷物配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	TSubclassOf<ACombatProjectile> ProjectileClass;

	// 发射位置Socket名称
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	FName ProjectileSocketName = TEXT("hand_r");

	// 蓄力影响曲线
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	UCurveFloat* ChargeSpeedCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	UCurveFloat* ChargeDamageCurve;

	// 蓄力时间记录
	float ChargeStartTime = 0.0f;

public:
	// 重写蓄力攻击结束，改为发射投掷物
	virtual void DoChargedAttackEnd() override;

	// 重写蓄力攻击开始，记录开始时间
	virtual void DoChargedAttackStart() override;

	// 发射投掷物
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Combat")
	void LaunchProjectile();

protected:

	// 计算蓄力倍率
	float GetChargeMultiplier(float ChargeTime) const;

	// 获取发射位置和方向
	void GetProjectileLaunchParams(FVector& OutLocation, FRotator& OutRotation) const;

	// 蓝图事件 - 投掷物发射时
	UFUNCTION(BlueprintImplementableEvent, Category="ElementalCombat|Combat")
	void OnProjectileLaunched(ACombatProjectile* Projectile);
};