// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalCombatGameInstance.h"
#include "Combat/Elemental/ElementalDataAsset.h"
#include "Combat/Elemental/ElementalConfigManager.h"
#include "Combat/Elemental/DefaultElementalDataAsset.h"
#include "AI/Utility/UtilityAITypes.h"

UElementalCombatGameInstance::UElementalCombatGameInstance()
{
	DefaultElementalDataAssetClass = nullptr;
	DefaultElementalDataAssetInstance = nullptr;
	AIProfileDataTable = nullptr;
}

void UElementalCombatGameInstance::Init()
{
	Super::Init();

	// 获取元素配置管理器
	UElementalConfigManager* ConfigManager = GetSubsystem<UElementalConfigManager>();
	if (!ConfigManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ElementalConfigManager not available!"));
		return;
	}

	// 创建默认元素数据资产实例
	if (DefaultElementalDataAssetClass)
	{
		DefaultElementalDataAssetInstance = NewObject<UElementalDataAsset>(this, DefaultElementalDataAssetClass);
		ConfigManager->SetElementalDataAsset(DefaultElementalDataAssetInstance);
		UE_LOG(LogTemp, Log, TEXT("全局元素配置已加载: %s"), *DefaultElementalDataAssetInstance->GetName());
	}
	else
	{
		// 如果没有指定配置类，创建默认配置
		DefaultElementalDataAssetInstance = NewObject<UDefaultElementalDataAsset>(this);
		ConfigManager->SetElementalDataAsset(DefaultElementalDataAssetInstance);
		UE_LOG(LogTemp, Warning, TEXT("未配置DefaultElementalDataAssetClass，使用内置默认配置"));
	}
}

void UElementalCombatGameInstance::SetDefaultElementalDataAsset(UElementalDataAsset* NewDataAsset)
{
	DefaultElementalDataAssetInstance = NewDataAsset;

	// 立即更新配置管理器
	if (UElementalConfigManager* ConfigManager = GetSubsystem<UElementalConfigManager>())
	{
		ConfigManager->SetElementalDataAsset(NewDataAsset);
		UE_LOG(LogTemp, Log, TEXT("运行时更新全局元素配置: %s"), 
			   NewDataAsset ? *NewDataAsset->GetName() : TEXT("NULL"));
	}
}

FUtilityProfile UElementalCombatGameInstance::GetRandomAIProfile() const
{
	if (!AIProfileDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI配置数据表未设置，使用默认配置"));
		return FUtilityProfile(); // 返回默认配置
	}

	// 获取数据表中的所有行名
	TArray<FName> AllRowNames = AIProfileDataTable->GetRowNames();
	if (AllRowNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI配置数据表为空，使用默认配置"));
		return FUtilityProfile(); // 返回默认配置
	}

	// 随机选择一行
	int32 RandomIndex = FMath::RandRange(0, AllRowNames.Num() - 1);
	FName SelectedRowName = AllRowNames[RandomIndex];

	// 获取选中行的数据
	FUtilityProfileTableRow* ProfileRow = AIProfileDataTable->FindRow<FUtilityProfileTableRow>(
		SelectedRowName, TEXT("GetRandomAIProfile"));

	if (ProfileRow)
	{
		UE_LOG(LogTemp, Log, TEXT("随机选择AI配置: %s (%s)"), 
			   *SelectedRowName.ToString(), *ProfileRow->Profile.ProfileName);
		return ProfileRow->Profile;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("无法加载AI配置行: %s"), *SelectedRowName.ToString());
		return FUtilityProfile(); // 返回默认配置
	}
}