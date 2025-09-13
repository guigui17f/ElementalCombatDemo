// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "ElementalCombatGameInstance.generated.h"

class UElementalDataAsset;
class UElementalConfigManager;
struct FUtilityProfile;

/**
 * 元素战斗游戏实例
 * 负责初始化全局元素配置管理器
 */
UCLASS(BlueprintType, Blueprintable)
class ELEMENTALCOMBAT_API UElementalCombatGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UElementalCombatGameInstance();

protected:
	// 默认元素数据配置类（在蓝图中设置）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat")
	TSubclassOf<UElementalDataAsset> DefaultElementalDataAssetClass;

	// 默认元素数据实例（运行时创建）
	UPROPERTY(Transient)
	UElementalDataAsset* DefaultElementalDataAssetInstance;

	// AI配置数据表（FUtilityProfileTableRow结构）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI Configuration")
	UDataTable* AIProfileDataTable;

public:
	// 重写初始化方法
	virtual void Init() override;

	/**
	 * 获取默认元素数据资产
	 * @return 配置的默认元素数据资产，可能为nullptr
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat")
	UElementalDataAsset* GetDefaultElementalDataAsset() const { return DefaultElementalDataAssetInstance; }

	/**
	 * 设置默认元素数据资产（运行时修改）
	 * @param NewDataAsset 新的元素数据资产
	 */
	UFUNCTION(BlueprintCallable, Category="ElementalCombat")
	void SetDefaultElementalDataAsset(UElementalDataAsset* NewDataAsset);

	/**
	 * 从配置表中随机获取一个AI配置
	 * @return 随机选择的AI配置，如果表为空返回默认配置
	 */
	UFUNCTION(BlueprintCallable, Category="AI Configuration")
	FUtilityProfile GetRandomAIProfile() const;
};