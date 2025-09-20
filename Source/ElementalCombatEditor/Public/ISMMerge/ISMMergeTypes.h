#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ISMMergeTypes.generated.h"

USTRUCT(BlueprintType)
struct ELEMENTALCOMBATEDITOR_API FISMMergeSettings
{
	GENERATED_BODY()

	/** 是否替换源Actors */
	UPROPERTY(EditAnywhere, Category = "合并选项")
	bool bReplaceSourceActors = true;

	/** 是否删除源Actors（如果为false，则仅隐藏） */
	UPROPERTY(EditAnywhere, Category = "合并选项", meta = (EditCondition = "bReplaceSourceActors"))
	bool bDeleteSourceActors = false;

	/** 是否保留源Actors的材质覆盖 */
	UPROPERTY(EditAnywhere, Category = "合并选项")
	bool bPreserveMaterialOverrides = true;

	/** 是否保留源组件的自定义数据 */
	UPROPERTY(EditAnywhere, Category = "合并选项")
	bool bPreserveCustomData = true;

	FISMMergeSettings()
	{
		bReplaceSourceActors = true;
		bDeleteSourceActors = false;
		bPreserveMaterialOverrides = true;
		bPreserveCustomData = true;
	}
};

USTRUCT()
struct ELEMENTALCOMBATEDITOR_API FISMCompatibilityResult
{
	GENERATED_BODY()

	/** 静态网格体组件是否与ISM组件兼容 */
	bool bIsCompatible = false;

	/** 不兼容的原因（如果有） */
	FString IncompatibilityReason;

	/** 可以接受静态网格体的目标ISM组件 */
	TWeakObjectPtr<UInstancedStaticMeshComponent> TargetComponent;

	FISMCompatibilityResult()
	{
		bIsCompatible = false;
	}

	FISMCompatibilityResult(bool InCompatible, const FString& InReason = TEXT(""))
		: bIsCompatible(InCompatible)
		, IncompatibilityReason(InReason)
	{
	}
};

USTRUCT()
struct ELEMENTALCOMBATEDITOR_API FISMActorData
{
	GENERATED_BODY()

	/** 包含ISM组件的Actor */
	TWeakObjectPtr<AActor> Actor;

	/** 此Actor中的ISM组件 */
	TArray<TWeakObjectPtr<UInstancedStaticMeshComponent>> ISMComponents;

	/** 此Actor是否被选中进行合并 */
	bool bIsSelected = true;

	FISMActorData()
	{
		bIsSelected = true;
	}

	FISMActorData(AActor* InActor)
		: Actor(InActor)
		, bIsSelected(true)
	{
		if (InActor)
		{
			TArray<UInstancedStaticMeshComponent*> Components;
			InActor->GetComponents<UInstancedStaticMeshComponent>(Components);
			for (UInstancedStaticMeshComponent* Component : Components)
			{
				ISMComponents.Add(Component);
			}
		}
	}
};

USTRUCT()
struct ELEMENTALCOMBATEDITOR_API FStaticMeshActorData
{
	GENERATED_BODY()

	/** 静态网格体Actor */
	TWeakObjectPtr<AActor> Actor;

	/** 静态网格体组件 */
	TWeakObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	/** 此Actor是否被选中进行合并 */
	bool bIsSelected = true;

	/** 与可用ISM组件的兼容性结果 */
	FISMCompatibilityResult CompatibilityResult;

	FStaticMeshActorData()
	{
		bIsSelected = true;
	}

	FStaticMeshActorData(AActor* InActor)
		: Actor(InActor)
		, bIsSelected(true)
	{
		if (InActor)
		{
			StaticMeshComponent = InActor->FindComponentByClass<UStaticMeshComponent>();
		}
	}
};