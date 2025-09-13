// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/AI/CombatStateTreeUtility.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "StateTreePlayerInfoTasks.generated.h"

/**
 * 获取玩家信息扩展任务实例数据
 * 继承自FStateTreeGetPlayerInfoInstanceData，增加水平夹角计算
 */
USTRUCT()
struct FStateTreePlayerInfoExtendedInstanceData : public FStateTreeGetPlayerInfoInstanceData
{
    GENERATED_BODY()

    /** 当前Actor正方向与玩家方向的水平夹角（0-180度）（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    float HorizontalAngleToPlayer = 0.0f;
};

STATETREE_POD_INSTANCEDATA(FStateTreePlayerInfoExtendedInstanceData);

/**
 * 获取玩家信息扩展任务
 * 继承自FStateTreeGetPlayerInfoTask功能，额外计算水平夹角
 */
USTRUCT(meta=(DisplayName="Get Player Info Extended", Category="ElementalCombat|Player Info"))
struct ELEMENTALCOMBAT_API FStateTreePlayerInfoExtendedTask : public FStateTreeGetPlayerInfoTask
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreePlayerInfoExtendedInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreePlayerInfoExtendedTask() = default;

    /** 持续运行时更新玩家信息和计算夹角 */
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};