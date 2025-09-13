// Copyright 2025 guigui17f. All Rights Reserved.

#include "StateTreePlayerInfoTasks.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Character.h"

EStateTreeRunStatus FStateTreePlayerInfoExtendedTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    // 首先调用父类的Tick函数来获取玩家信息
    EStateTreeRunStatus ParentResult = Super::Tick(Context, DeltaTime);

    // 获取实例数据
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    // 计算水平夹角（只有在有有效玩家角色和拥有角色时才计算）
    if (InstanceData.TargetPlayerCharacter && InstanceData.Character)
    {
        // 获取从当前Actor到玩家的方向向量
        FVector DirectionToPlayer = (InstanceData.TargetPlayerLocation - InstanceData.Character->GetActorLocation());

        // 投影到水平面（忽略Z轴）
        DirectionToPlayer.Z = 0.0f;
        DirectionToPlayer.Normalize();

        // 获取Actor的前向向量并投影到水平面
        FVector ActorForward = InstanceData.Character->GetActorForwardVector();
        ActorForward.Z = 0.0f;
        ActorForward.Normalize();

        // 使用点积计算夹角（0-180度）
        float DotProduct = FVector::DotProduct(ActorForward, DirectionToPlayer);
        float AngleRadians = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
        InstanceData.HorizontalAngleToPlayer = FMath::RadiansToDegrees(AngleRadians);
    }
    else
    {
        // 没有有效数据时重置夹角
        InstanceData.HorizontalAngleToPlayer = 0.0f;
    }

    return ParentResult;
}

#if WITH_EDITOR
FText FStateTreePlayerInfoExtendedTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
    return FText::FromString(TEXT("Get Player Info Extended"));
}
#endif // WITH_EDITOR