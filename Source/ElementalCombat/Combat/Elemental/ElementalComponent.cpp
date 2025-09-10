// Copyright 2025 guigui17f. All Rights Reserved.

#include "Combat/Elemental/ElementalComponent.h"
#include "Combat/Elemental/ElementalDataAsset.h"
#include "Combat/Elemental/ElementalConfigManager.h"
#include "Combat/Elemental/DefaultElementalDataAsset.h"

UElementalComponent::UElementalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentElement = EElementalType::None;
	ElementalDataAsset = nullptr;
}

void UElementalComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 从全局配置管理器获取配置
	if (UElementalConfigManager* ConfigManager = UElementalConfigManager::GetInstance(this))
	{
		ElementalDataAsset = ConfigManager->GetElementalDataAsset();
	}
	
	// 如果没有全局配置，使用默认配置
	if (!ElementalDataAsset)
	{
		ElementalDataAsset = NewObject<UDefaultElementalDataAsset>(this);
		UE_LOG(LogTemp, Warning, TEXT("ElementalComponent: 使用默认元素配置 - %s"), *GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ElementalComponent: 使用全局元素配置 - %s"), *ElementalDataAsset->GetName());
	}
	
	// 从ElementalDataAsset加载数据到运行时缓存
	RefreshFromDataAsset();
}

void UElementalComponent::SwitchElement(EElementalType NewElement)
{
	if (CurrentElement != NewElement)
	{
		EElementalType PreviousElement = CurrentElement;
		CurrentElement = NewElement;
		
		// 广播元素变更事件
		BroadcastElementChanged(NewElement);
	}
}

void UElementalComponent::SetElementEffectData(EElementalType Element, const FElementalEffectData& EffectData)
{
	ElementEffectDataMap.Add(Element, EffectData);
}

bool UElementalComponent::GetElementEffectData(EElementalType Element, FElementalEffectData& OutEffectData) const
{
	const FElementalEffectData* FoundData = GetElementEffectDataPtr(Element);
	if (FoundData)
	{
		OutEffectData = *FoundData;
		return true;
	}
	
	return false;
}

const FElementalEffectData* UElementalComponent::GetElementEffectDataPtr(EElementalType Element) const
{
	return ElementEffectDataMap.Find(Element);
}

UClass* UElementalComponent::GetCurrentProjectileClass() const
{
	const FElementalEffectData* CurrentData = GetElementEffectDataPtr(CurrentElement);
	if (CurrentData && CurrentData->ProjectileClass)
	{
		return CurrentData->ProjectileClass;
	}
	
	return nullptr;
}

bool UElementalComponent::HasElementData(EElementalType Element) const
{
	return ElementEffectDataMap.Contains(Element);
}

void UElementalComponent::RefreshFromDataAsset()
{
	if (ElementalDataAsset)
	{
		// 清空现有数据
		ElementEffectDataMap.Empty();
		
		// 从DataAsset复制数据到组件
		ElementalDataAsset->CopyDataToComponent(this);
		
		UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Loaded data from ElementalDataAsset"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementalComponent: No ElementalDataAsset assigned"));
	}
}

void UElementalComponent::BroadcastElementChanged(EElementalType NewElement)
{
	if (OnElementChanged.IsBound())
	{
		OnElementChanged.Broadcast(NewElement);
	}
}