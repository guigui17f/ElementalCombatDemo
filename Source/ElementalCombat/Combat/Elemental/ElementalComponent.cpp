// Copyright 2025 guigui17f. All Rights Reserved.

#include "Combat/Elemental/ElementalComponent.h"

UElementalComponent::UElementalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentElement = EElementalType::None;
}

void UElementalComponent::BeginPlay()
{
	Super::BeginPlay();
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

void UElementalComponent::BroadcastElementChanged(EElementalType NewElement)
{
	if (OnElementChanged.IsBound())
	{
		OnElementChanged.Broadcast(NewElement);
	}
}