// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/CombatCharacter.h"
#include "Combat/Projectiles/CombatProjectile.h"
#include "Combat/Elemental/ElementalTypes.h"
#include "AdvancedCombatCharacter.generated.h"

class UCurveFloat;
class UElementalComponent;
class UInputAction;
class UCombatLifeBar;
class UAnimMontage;

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
	// 元素组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UElementalComponent* ElementalComponent;

	// 投掷物配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Characters")
	TSubclassOf<ACombatProjectile> ProjectileClass;

	// 发射位置Socket名称
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Characters")
	FName ProjectileSocketName = TEXT("hand_r");

	// 远程攻击动画蒙太奇
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UAnimMontage* RangedAttackMontage;

	// 蓄力影响曲线
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UCurveFloat* ChargeSpeedCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UCurveFloat* ChargeDamageCurve;

	// 蓄力时间记录
	float ChargeStartTime = 0.0f;

	// 元素切换输入动作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UInputAction* SwitchToMetalAction;  // 1键 - 金

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UInputAction* SwitchToWoodAction;   // 2键 - 木

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UInputAction* SwitchToWaterAction;  // 3键 - 水

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UInputAction* SwitchToFireAction;   // 4键 - 火

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ElementalCombat|Characters")
	UInputAction* SwitchToEarthAction;  // 5键 - 土

public:
	// 重写蓄力攻击结束，改为发射投掷物
	virtual void DoChargedAttackEnd() override;

	// 重写蓄力攻击开始，记录开始时间
	virtual void DoChargedAttackStart() override;

	// 发射投掷物
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Characters")
	void LaunchProjectile();

	// 元素切换函数
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Characters")
	void SwitchToElement(EElementalType NewElement);

	// 各元素的快捷切换
	void SwitchToMetal() { SwitchToElement(EElementalType::Metal); }
	void SwitchToWood() { SwitchToElement(EElementalType::Wood); }
	void SwitchToWater() { SwitchToElement(EElementalType::Water); }
	void SwitchToFire() { SwitchToElement(EElementalType::Fire); }
	void SwitchToEarth() { SwitchToElement(EElementalType::Earth); }

	// 重写输入绑定
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 重写受击方法，移除ragdoll效果
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent,
	                        AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Landed(const FHitResult& Hit) override;

	// 重写治疗方法，实现生命恢复
	virtual void ApplyHealing(float Healing, AActor* Healer) override;

protected:
	// 初始化
	virtual void BeginPlay() override;

	// 计算蓄力倍率
	float GetChargeMultiplier(float ChargeTime) const;

	// 获取发射位置和方向
	void GetProjectileLaunchParams(FVector& OutLocation, FRotator& OutRotation) const;

	// 更新材质颜色
	void UpdateMaterialColors(FLinearColor Color);

	// 执行远程攻击动画
	void DoRangedAttack();

	// 蓝图事件 - 投掷物发射时
	UFUNCTION(BlueprintImplementableEvent, Category="ElementalCombat|Characters")
	void OnProjectileLaunched(ACombatProjectile* Projectile);
};