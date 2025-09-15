// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/AI/CombatEnemy.h"
#include "ElementalCombatEnemy.generated.h"

class ACombatProjectile;
class UCombatLifeBar;

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
	// 元素组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ElementalCombat|AI")
	class UElementalComponent* ElementalComponent;

	// AI攻击距离配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|AI")
	float MeleeAttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|AI")
	float RangedAttackRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|AI")
	float PreferredAttackRange = 500.0f;

	// AI使用的投掷物类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|AI")
	TSubclassOf<ACombatProjectile> AIProjectileClass;

	// 投掷物发射Socket
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|AI")
	FName ProjectileSocketName = TEXT("hand_r");

	// 远程攻击动画
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|AI")
	UAnimMontage* RangedAttackMontage;

public:
	// 当前攻击类型
	UPROPERTY(BlueprintReadOnly, Category="ElementalCombat|AI")
	EAIAttackType CurrentAttackType = EAIAttackType::None;

public:
	// AI攻击决策
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|AI")
	EAIAttackType DecideAttackType(float DistanceToTarget) const;

	// 检查是否正在攻击
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|Combat")
	bool IsAttacking() const { return bIsAttacking; }

	// 检查是否在偏好攻击距离
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|AI")
	bool IsInPreferredRange(float DistanceToTarget) const;

	// AI发起远程攻击
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|AI")
	void DoAIRangedAttack();

	// 发射投掷物（由动画通知调用）
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|AI")
	void LaunchProjectile();

	// 获取目标距离
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|AI")
	float GetDistanceToTarget() const;

	// 获取远程攻击范围
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|AI")
	float GetRangedAttackRange() const { return RangedAttackRange; }

	// 重写受击方法，移除ragdoll效果
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent,
	                        AController* EventInstigator, AActor* DamageCauser) override;

	// 重写治疗方法，实现生命恢复
	virtual void ApplyHealing(float Healing, AActor* Healer) override;

protected:
	// 获取发射参数
	void GetProjectileLaunchParams(FVector& OutLocation, FRotator& OutRotation) const;

	// 蓝图事件 - 远程攻击开始
	UFUNCTION(BlueprintImplementableEvent, Category="ElementalCombat|AI")
	void OnRangedAttackStarted();

	// 蓝图事件 - 投掷物发射
	UFUNCTION(BlueprintImplementableEvent, Category="ElementalCombat|AI")
	void OnProjectileLaunched(ACombatProjectile* Projectile);

	// 初始化
	virtual void BeginPlay() override;
};