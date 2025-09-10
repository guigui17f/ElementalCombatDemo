// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ElementalTypes.generated.h"

// Forward declaration
class ACombatProjectile;

/**
 * 五行元素类型枚举
 * 金木水火土 + None
 */
UENUM(BlueprintType)
enum class EElementalType : uint8
{
	None = 0		UMETA(DisplayName = "无元素"),
	Metal = 1		UMETA(DisplayName = "金"),
	Wood = 2		UMETA(DisplayName = "木"),
	Water = 3		UMETA(DisplayName = "水"),
	Fire = 4		UMETA(DisplayName = "火"),
	Earth = 5		UMETA(DisplayName = "土")
};

/**
 * 元素效果数据结构
 * 包含所有元素的效果参数，通过蓝图配置
 */
USTRUCT(BlueprintType)
struct ELEMENTALCOMBAT_API FElementalEffectData
{
	GENERATED_BODY()

	FElementalEffectData()
	{
		DamageMultiplier = 1.0f;
		LifeStealPercentage = 0.0f;
		SlowPercentage = 0.0f;
		SlowDuration = 0.0f;
		DotDamage = 0.0f;
		DotTickInterval = 1.0f;
		DotDuration = 0.0f;
		DamageReduction = 0.0f;
		ProjectileClass = nullptr;
	}

	// 金元素 - 伤害倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.0"))
	float DamageMultiplier;

	// 木元素 - 吸血比例
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LifeStealPercentage;

	// 水元素 - 减速比例和持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlowPercentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.0"))
	float SlowDuration;

	// 火元素 - DOT伤害
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.0"))
	float DotDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.1"))
	float DotTickInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.0"))
	float DotDuration;

	// 土元素 - 减伤比例
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageReduction;

	// 投掷物类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental")
	UClass* ProjectileClass;
};

/**
 * 元素相克数据
 * 定义某个元素克制的目标元素和效果倍率
 */
USTRUCT(BlueprintType)
struct ELEMENTALCOMBAT_API FElementalCounterData
{
	GENERATED_BODY()

	FElementalCounterData()
	{
		CounteredElement = EElementalType::None;
		EffectMultiplier = 1.0f;
	}

	// 被克制的元素
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental")
	EElementalType CounteredElement;

	// 克制效果倍率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental", meta = (ClampMin = "0.0"))
	float EffectMultiplier;
};

/**
 * 元素关系配置
 * 定义某个元素的所有克制关系
 */
USTRUCT(BlueprintType)
struct ELEMENTALCOMBAT_API FElementalRelationship
{
	GENERATED_BODY()

	FElementalRelationship()
	{
		Element = EElementalType::None;
	}

	// 当前元素
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental")
	EElementalType Element;

	// 克制关系列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|Combat|Elemental")
	TArray<FElementalCounterData> Counters;
};