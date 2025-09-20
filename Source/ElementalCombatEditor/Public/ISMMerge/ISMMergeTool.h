#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/World.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ISMMergeTypes.h"
#include "ISMMergeTool.generated.h"

class SISMMergeWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogISMMergeTool, Log, All);

/**
 * ISM合并工具 - 用于将静态网格体Actors合并到ISM组件的核心功能
 */
UCLASS()
class ELEMENTALCOMBATEDITOR_API UISMMergeTool : public UObject
{
	GENERATED_BODY()

public:
	UISMMergeTool();

	/** 获取单例实例 */
	static UISMMergeTool* Get();

	/** 执行合并操作 */
	bool ExecuteMerge(const TArray<FISMActorData>& ISMActors,
					 const TArray<FStaticMeshActorData>& StaticMeshActors,
					 const FISMMergeSettings& Settings);

	/** 检查静态网格体组件与ISM组件之间的兼容性 */
	FISMCompatibilityResult CheckISMCompatibility(UStaticMeshComponent* SourceComponent,
												 UInstancedStaticMeshComponent* TargetComponent) const;

	/** 为静态网格体组件找到最佳兼容的ISM组件 */
	UInstancedStaticMeshComponent* FindBestISMTarget(UStaticMeshComponent* SourceComponent,
													const TArray<FISMActorData>& ISMActors) const;

	/** 从当前选择中收集所有ISM Actors */
	TArray<FISMActorData> CollectISMActorsFromSelection() const;

	/** 从当前选择中收集所有静态网格体Actors */
	TArray<FStaticMeshActorData> CollectStaticMeshActorsFromSelection() const;

	/** 获取合并设置 */
	const FISMMergeSettings& GetSettings() const { return MergeSettings; }

	/** 设置合并设置 */
	void SetSettings(const FISMMergeSettings& InSettings) { MergeSettings = InSettings; }

private:
	/** 安全移除Actor并进行适当的引用清理 */
	void SafelyRemoveActor(AActor* Actor, bool bDeleteActor);

	/** 在删除Actor前清理对它的引用 */
	void CleanupActorReferences(AActor* ActorToRemove);

	/** 将引用从旧Actor更新到新Actor */
	void UpdateReferences(const TArray<UObject*>& Referencers, AActor* OldActor, AActor* NewActor);

	/** 收集所有引用给定Actor的对象 */
	void CollectActorReferences(AActor* Actor, TArray<UObject*>& OutReferencers);

	/** 处理OFPA特定的Actor删除 */
	void HandleOFPAActorDeletion(AActor* Actor);

private:
	/** 当前合并设置 */
	UPROPERTY()
	FISMMergeSettings MergeSettings;

	/** 静态实例 */
	static UISMMergeTool* Instance;
};