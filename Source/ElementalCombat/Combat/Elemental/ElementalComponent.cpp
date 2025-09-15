// Copyright 2025 guigui17f. All Rights Reserved.

#include "Combat/Elemental/ElementalComponent.h"
#include "Combat/Elemental/ElementalDataAsset.h"
#include "Combat/Elemental/ElementalConfigManager.h"
#include "Combat/Elemental/DefaultElementalDataAsset.h"
#include "Combat/Elemental/ElementalCalculator.h"
#include "Combat/Elemental/ElementalEffectProcessor.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

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

// ========== 数据驱动的元素效果处理实现 ==========

float UElementalComponent::ProcessElementalDamage(
	float BaseDamage,
	const FElementalEffectData& AttackerEffectData,
	AActor* DamageCauser)
{
	float FinalDamage = BaseDamage;

	// 1. 应用攻击方的伤害倍率（如果配置了）
	if (AttackerEffectData.DamageMultiplier != 1.0f && AttackerEffectData.DamageMultiplier > 0.0f)
	{
		FinalDamage *= AttackerEffectData.DamageMultiplier;
		UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Applied damage multiplier %.2f to %s"),
			AttackerEffectData.DamageMultiplier, *GetOwner()->GetName());
	}

	// 2. 计算元素相克（使用现有的Calculator）
	EElementalType AttackerElement = AttackerEffectData.Element;
	if (AttackerElement != EElementalType::None && CurrentElement != EElementalType::None)
	{
		float CounterMultiplier = UElementalCalculator::CalculateCounterMultiplier(
			AttackerElement, CurrentElement, GetOwner());
		if (CounterMultiplier != 1.0f)
		{
			FinalDamage *= CounterMultiplier;
			UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Applied elemental counter multiplier %.2f to %s"),
				CounterMultiplier, *GetOwner()->GetName());
		}
	}

	// 3. 应用防御方的减伤（如果自己有减伤配置）
	const FElementalEffectData* DefenderData = GetElementEffectDataPtr(CurrentElement);
	if (DefenderData && DefenderData->DamageReduction > 0.0f)
	{
		float ReductionRatio = FMath::Clamp(DefenderData->DamageReduction, 0.0f, 1.0f);
		FinalDamage *= (1.0f - ReductionRatio);
		UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Applied damage reduction %.1f%% to %s"),
			ReductionRatio * 100.0f, *GetOwner()->GetName());
	}

	return FMath::Max(FinalDamage, 0.0f);
}

void UElementalComponent::ApplyElementalEffects(
	const FElementalEffectData& EffectData,
	AActor* EffectCauser,
	float DamageDealt)
{
	// 检查每个字段，如果有效则应用对应效果

	// 减速效果 - 检查SlowPercentage和SlowDuration
	if (EffectData.SlowPercentage > 0.0f && EffectData.SlowDuration > 0.0f)
	{
		ApplySlowIfConfigured(EffectData);
	}

	// DOT效果 - 检查DotDamage和DotDuration
	if (EffectData.DotDamage > 0.0f && EffectData.DotDuration > 0.0f)
	{
		ApplyDotIfConfigured(EffectData, EffectCauser);
	}

	// 吸血效果 - 检查LifeStealPercentage
	if (EffectData.LifeStealPercentage > 0.0f && DamageDealt > 0.0f)
	{
		ApplyLifeStealIfConfigured(DamageDealt, EffectData, EffectCauser);
	}

	// 注意：DamageMultiplier和DamageReduction在ProcessElementalDamage中处理
}

void UElementalComponent::ApplySlowIfConfigured(const FElementalEffectData& EffectData)
{
	// 不检查元素类型，只看配置值
	if (EffectData.SlowPercentage <= 0.0f || EffectData.SlowDuration <= 0.0f)
		return;

	if (bIsSlowed)
	{
		// 刷新减速效果 - 清除旧的定时器
		GetWorld()->GetTimerManager().ClearTimer(SlowEffectTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Refreshing slow effect on %s"), *GetOwner()->GetName());
	}

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			if (!bIsSlowed)
			{
				OriginalWalkSpeed = Movement->MaxWalkSpeed;
			}

			// 使用ElementalEffectProcessor计算减速后的速度
			float SlowedSpeed = UElementalEffectProcessor::CalculateSlowedSpeed(
				OriginalWalkSpeed, EffectData);
			Movement->MaxWalkSpeed = SlowedSpeed;

			bIsSlowed = true;

			UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Applied slow effect to %s (%.0f -> %.0f for %.1fs)"),
				*GetOwner()->GetName(), OriginalWalkSpeed, SlowedSpeed, EffectData.SlowDuration);

			// 设置恢复定时器
			GetWorld()->GetTimerManager().SetTimer(
				SlowEffectTimerHandle,
				this,
				&UElementalComponent::OnSlowEffectEnd,
				EffectData.SlowDuration,
				false
			);
		}
	}
}

void UElementalComponent::ApplyDotIfConfigured(const FElementalEffectData& EffectData, AActor* Causer)
{
	// 不检查元素类型，只看配置值
	if (EffectData.DotDamage <= 0.0f || EffectData.DotDuration <= 0.0f)
		return;

	// 如果已经在DOT，刷新效果
	if (bIsBurning)
	{
		GetWorld()->GetTimerManager().ClearTimer(DotEffectTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Refreshing DOT effect on %s"), *GetOwner()->GetName());
	}

	// 计算tick次数
	int32 TotalTicks = UElementalEffectProcessor::CalculateDotTicks(EffectData);
	if (TotalTicks <= 0) return;

	// 设置DOT参数
	DotDamagePerTick = EffectData.DotDamage;
	RemainingDotTicks = TotalTicks;
	DotCauser = Causer;
	bIsBurning = true;

	UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Applied DOT effect to %s (%.1f damage x %d ticks, interval %.1fs)"),
		*GetOwner()->GetName(), EffectData.DotDamage, TotalTicks, EffectData.DotTickInterval);

	// 启动DOT定时器
	float TickInterval = FMath::Max(EffectData.DotTickInterval, 0.1f);
	GetWorld()->GetTimerManager().SetTimer(
		DotEffectTimerHandle,
		this,
		&UElementalComponent::OnDotEffectTick,
		TickInterval,
		true
	);
}

void UElementalComponent::ApplyLifeStealIfConfigured(
	float DamageDealt,
	const FElementalEffectData& EffectData,
	AActor* Attacker)
{
	// 不检查元素类型，只看配置值
	if (EffectData.LifeStealPercentage <= 0.0f || DamageDealt <= 0.0f || !Attacker)
		return;

	float HealAmount = UElementalEffectProcessor::CalculateLifeSteal(DamageDealt, EffectData);

	if (HealAmount > 0.0f)
	{
		if (ICombatDamageable* AttackerDamageable = Cast<ICombatDamageable>(Attacker))
		{
			AttackerDamageable->ApplyHealing(HealAmount, Attacker);
			UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Applied life steal to %s (%.1f healing from %.1f damage)"),
				*Attacker->GetName(), HealAmount, DamageDealt);
		}
	}
}

// ========== 效果结束回调 ==========

void UElementalComponent::OnSlowEffectEnd()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = OriginalWalkSpeed;
			UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Slow effect ended on %s (restored to %.0f)"),
				*GetOwner()->GetName(), OriginalWalkSpeed);
		}
	}
	bIsSlowed = false;
}

void UElementalComponent::OnDotEffectTick()
{
	if (RemainingDotTicks <= 0)
	{
		bIsBurning = false;
		GetWorld()->GetTimerManager().ClearTimer(DotEffectTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("ElementalComponent: DOT effect ended on %s"), *GetOwner()->GetName());
		return;
	}

	// 应用DOT伤害
	if (ICombatDamageable* Damageable = Cast<ICombatDamageable>(GetOwner()))
	{
		Damageable->ApplyDamage(DotDamagePerTick, DotCauser.Get(),
			GetOwner()->GetActorLocation(), FVector::ZeroVector);
		UE_LOG(LogTemp, Verbose, TEXT("ElementalComponent: DOT tick on %s (%.1f damage, %d ticks remaining)"),
			*GetOwner()->GetName(), DotDamagePerTick, RemainingDotTicks - 1);
	}

	RemainingDotTicks--;
}

void UElementalComponent::ClearAllEffects()
{
	// 清除减速效果
	if (bIsSlowed)
	{
		OnSlowEffectEnd();
		GetWorld()->GetTimerManager().ClearTimer(SlowEffectTimerHandle);
	}

	// 清除DOT效果
	if (bIsBurning)
	{
		bIsBurning = false;
		GetWorld()->GetTimerManager().ClearTimer(DotEffectTimerHandle);
		RemainingDotTicks = 0;
		DotCauser = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("ElementalComponent: Cleared all effects on %s"), *GetOwner()->GetName());
}