// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ElementalTypes.h"
#include "Engine/TimerHandle.h"
#include "ElementalComponent.generated.h"

class ACombatProjectile;
class UElementalDataAsset;

// 元素变更委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElementChanged, EElementalType, NewElement);

/**
 * 元素组件
 * 负责管理Actor的当前元素状态、元素切换、效果数据存储等功能
 */
UCLASS(ClassGroup=(ElementalCombat), meta=(BlueprintSpawnableComponent))
class ELEMENTALCOMBAT_API UElementalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UElementalComponent();

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * 切换到指定元素
	 * @param NewElement 新的元素类型
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|Combat|Elemental")
	void SwitchElement(EElementalType NewElement);

	/**
	 * 获取当前元素
	 * @return 当前元素类型
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ElementalCombat|Combat|Elemental")
	EElementalType GetCurrentElement() const { return CurrentElement; }

	/**
	 * 设置元素效果数据
	 * @param Element 元素类型
	 * @param EffectData 效果数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|Combat|Elemental")
	void SetElementEffectData(EElementalType Element, const FElementalEffectData& EffectData);

	/**
	 * 获取元素效果数据
	 * @param Element 元素类型
	 * @param OutEffectData 输出的效果数据
	 * @return true如果找到数据
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ElementalCombat|Combat|Elemental")
	bool GetElementEffectData(EElementalType Element, FElementalEffectData& OutEffectData) const;

	/**
	 * 获取元素效果数据（C++用）
	 * @param Element 元素类型
	 * @return 效果数据指针，如果不存在返回nullptr
	 */
	const FElementalEffectData* GetElementEffectDataPtr(EElementalType Element) const;

	/**
	 * 获取当前元素的投掷物类
	 * @return 投掷物类，如果没有设置返回nullptr
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ElementalCombat|Combat|Elemental")
	UClass* GetCurrentProjectileClass() const;

	/**
	 * 检查是否有指定元素的数据
	 * @param Element 元素类型
	 * @return true如果有该元素的数据
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ElementalCombat|Combat|Elemental")
	bool HasElementData(EElementalType Element) const;

	/**
	 * 从ElementalDataAsset刷新元素数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|Combat|Elemental")
	void RefreshFromDataAsset();

	/**
	 * 获取当前使用的ElementalDataAsset
	 * @return ElementalDataAsset引用
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ElementalCombat|Combat|Elemental")
	UElementalDataAsset* GetElementalDataAsset() const { return ElementalDataAsset; }

	// ========== 数据驱动的元素效果处理 ==========

	/**
	 * 处理元素伤害 - 完全基于数据配置
	 * @param BaseDamage 基础伤害
	 * @param AttackerEffectData 攻击者的效果数据
	 * @param DamageCauser 造成伤害的Actor
	 * @return 处理后的最终伤害
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|Combat|Elemental")
	float ProcessElementalDamage(
		float BaseDamage,
		const FElementalEffectData& AttackerEffectData,
		AActor* DamageCauser
	);

	/**
	 * 应用元素效果 - 基于数据字段
	 * @param EffectData 效果数据配置
	 * @param EffectCauser 造成效果的Actor
	 * @param DamageDealt 造成的伤害（用于吸血计算）
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|Combat|Elemental")
	void ApplyElementalEffects(
		const FElementalEffectData& EffectData,
		AActor* EffectCauser,
		float DamageDealt = 0.0f
	);

	// ========== 状态查询 ==========
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ElementalCombat|Combat|Elemental")
	bool IsSlowed() const { return bIsSlowed; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ElementalCombat|Combat|Elemental")
	bool IsBurning() const { return bIsBurning; }

	// ========== 测试支持 ==========
	/**
	 * 清除所有活跃的元素效果（主要用于测试）
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|Combat|Elemental")
	void ClearAllEffects();

public:
	// 元素变更委托
	UPROPERTY(BlueprintAssignable, Category = "ElementalCombat|Combat|Elemental")
	FOnElementChanged OnElementChanged;

protected:
	// 元素配置数据资产（由全局配置管理器提供，不再允许直接编辑）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElementalCombat|Combat|Elemental")
	UElementalDataAsset* ElementalDataAsset;

	// 当前元素
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElementalCombat|Combat|Elemental")
	EElementalType CurrentElement;

	// 元素效果数据映射（运行时缓存，从DataAsset加载）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElementalCombat|Combat|Elemental")
	TMap<EElementalType, FElementalEffectData> ElementEffectDataMap;

	// ========== 效果状态管理 ==========
	// 减速效果
	UPROPERTY(BlueprintReadOnly, Category = "ElementalCombat|Combat|Elemental")
	bool bIsSlowed = false;

	FTimerHandle SlowEffectTimerHandle;
	float OriginalWalkSpeed = 0.0f;

	// DOT效果
	UPROPERTY(BlueprintReadOnly, Category = "ElementalCombat|Combat|Elemental")
	bool bIsBurning = false;

	FTimerHandle DotEffectTimerHandle;
	float DotDamagePerTick = 0.0f;
	int32 RemainingDotTicks = 0;
	TWeakObjectPtr<AActor> DotCauser;

private:
	// 触发元素变更委托
	void BroadcastElementChanged(EElementalType NewElement);

	// 基于字段值的效果应用
	void ApplySlowIfConfigured(const FElementalEffectData& EffectData);
	void ApplyDotIfConfigured(const FElementalEffectData& EffectData, AActor* Causer);
	void ApplyLifeStealIfConfigured(float DamageDealt, const FElementalEffectData& EffectData, AActor* Attacker);

	// 效果结束回调
	void OnSlowEffectEnd();
	void OnDotEffectTick();
};