// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/AI/CombatEnemy.h"
#include "ElementalCombatEnemy.generated.h"

class ACombatProjectile;

/**
 * 攻击类型枚举
 */
UENUM(BlueprintType)
enum class EAIAttackType : uint8
{
	None = 0,
	Melee = 1,
	Ranged = 2
};

/**
 * 元素战斗敌人类
 * 支持近战和远程两种攻击模式
 * 设计为通用的元素战斗敌人基础类
 */
UCLASS()
class ELEMENTALCOMBAT_API AElementalCombatEnemy : public ACombatEnemy
{
	GENERATED_BODY()

public:
	AElementalCombatEnemy();

protected:
	// AI攻击距离配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float MeleeAttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float RangedAttackRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float PreferredAttackRange = 500.0f;

	// AI使用的投掷物类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	TSubclassOf<ACombatProjectile> AIProjectileClass;

	// 投掷物发射Socket
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	FName ProjectileSocketName = TEXT("hand_r");

	// 远程攻击动画
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	UAnimMontage* RangedAttackMontage;

public:
	// 当前攻击类型
	UPROPERTY(BlueprintReadOnly, Category="ElementalCombat|Combat")
	EAIAttackType CurrentAttackType = EAIAttackType::None;

protected:

public:
	// AI攻击决策
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Combat")
	EAIAttackType DecideAttackType(float DistanceToTarget) const;

	// 检查是否在偏好攻击距离
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|Combat")
	bool IsInPreferredRange(float DistanceToTarget) const;

	// AI发起远程攻击
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Combat")
	void DoAIRangedAttack();

	// 发射投掷物（由动画通知调用）
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Combat")
	void LaunchProjectile();

	// 获取目标距离
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|Combat")
	float GetDistanceToTarget() const;

protected:
	// 获取发射参数
	void GetProjectileLaunchParams(FVector& OutLocation, FRotator& OutRotation) const;

	// 蓝图事件 - 远程攻击开始
	UFUNCTION(BlueprintImplementableEvent, Category="ElementalCombat|Combat")
	void OnRangedAttackStarted();

	// 蓝图事件 - 投掷物发射
	UFUNCTION(BlueprintImplementableEvent, Category="ElementalCombat|Combat")
	void OnProjectileLaunched(ACombatProjectile* Projectile);

	// 初始化
	virtual void BeginPlay() override;
};