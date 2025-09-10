// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ElementalTypes.h"
#include "ElementalDataAsset.h"
#include "ElementalConfigManager.generated.h"

/**
 * 元素配置管理器
 * 单例模式，管理全局的元素配置数据
 * 为ElementalCalculator提供数据驱动的相克关系查询
 */
UCLASS()
class ELEMENTALCOMBAT_API UElementalConfigManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * 获取元素配置管理器实例
	 * @param WorldContextObject 世界上下文对象
	 * @return 配置管理器实例，如果获取失败返回nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Elemental Config")
	static UElementalConfigManager* GetInstance(const UObject* WorldContextObject);

	/**
	 * 设置当前使用的元素数据资产
	 * @param DataAsset 元素数据资产
	 */
	UFUNCTION(BlueprintCallable, Category = "Elemental Config")
	void SetElementalDataAsset(UElementalDataAsset* DataAsset);

	/**
	 * 获取当前使用的元素数据资产
	 * @return 当前数据资产，可能为nullptr
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental Config")
	UElementalDataAsset* GetElementalDataAsset() const { return CurrentDataAsset; }

	/**
	 * 查询元素是否克制另一个元素
	 * @param AttackerElement 攻击者元素
	 * @param DefenderElement 防御者元素
	 * @return true如果攻击者克制防御者
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental Config")
	bool IsElementAdvantage(EElementalType AttackerElement, EElementalType DefenderElement) const;

	/**
	 * 获取元素相克倍率
	 * @param AttackerElement 攻击者元素
	 * @param DefenderElement 防御者元素
	 * @return 相克倍率，如果没有配置返回默认值
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental Config")
	float GetCounterMultiplier(EElementalType AttackerElement, EElementalType DefenderElement) const;

	/**
	 * 获取指定元素的相克关系
	 * @param Element 元素类型
	 * @param OutRelationship 输出的相克关系
	 * @return true如果找到关系
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental Config")
	bool GetElementRelationship(EElementalType Element, FElementalRelationship& OutRelationship) const;

	/**
	 * 检查是否有有效的配置数据
	 * @return true如果有有效配置
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental Config")
	bool HasValidConfiguration() const;

protected:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// 当前使用的元素数据资产
	UPROPERTY()
	UElementalDataAsset* CurrentDataAsset;

	// 默认相克关系倍率
	static constexpr float DEFAULT_ADVANTAGE_MULTIPLIER = 1.5f;
	static constexpr float DEFAULT_DISADVANTAGE_MULTIPLIER = 0.5f;
	static constexpr float DEFAULT_NEUTRAL_MULTIPLIER = 1.0f;

	/**
	 * 使用硬编码逻辑判断元素克制关系（后备方案）
	 * @param AttackerElement 攻击者元素
	 * @param DefenderElement 防御者元素
	 * @return true如果攻击者克制防御者
	 */
	bool IsElementAdvantageDefault(EElementalType AttackerElement, EElementalType DefenderElement) const;
};