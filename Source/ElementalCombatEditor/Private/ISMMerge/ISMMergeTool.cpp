#include "ISMMerge/ISMMergeTool.h"
#include "Engine/Selection.h"
#include "Editor.h"
#include "EditorModeManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Engine/LevelStreaming.h"
#include "Algo/AllOf.h"

DEFINE_LOG_CATEGORY(LogISMMergeTool);

#define LOCTEXT_NAMESPACE "ISMMergeTool"

UISMMergeTool* UISMMergeTool::Instance = nullptr;

UISMMergeTool::UISMMergeTool()
{
	Instance = this;
}

UISMMergeTool* UISMMergeTool::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UISMMergeTool>();
		Instance->AddToRoot();
	}
	return Instance;
}

bool UISMMergeTool::ExecuteMerge(const TArray<FISMActorData>& ISMActors,
								const TArray<FStaticMeshActorData>& StaticMeshActors,
								const FISMMergeSettings& Settings)
{
	FScopedTransaction Transaction(LOCTEXT("ISMMergeTransaction", "Merge Static Meshes to ISM"));

	int32 MergedCount = 0;
	int32 ISMCount = 0;
	TMap<AActor*, TArray<UObject*>> ActorReferences;

	UE_LOG(LogISMMergeTool, Display, TEXT("开始ISM合并操作..."));

	// Collect references for actors that will be deleted
	if (Settings.bReplaceSourceActors)
	{
		for (const FStaticMeshActorData& ActorData : StaticMeshActors)
		{
			if (ActorData.bIsSelected && ActorData.Actor.IsValid())
			{
				CollectActorReferences(ActorData.Actor.Get(), ActorReferences.FindOrAdd(ActorData.Actor.Get()));
			}
		}
	}

	// Process each selected static mesh actor
	for (const FStaticMeshActorData& ActorData : StaticMeshActors)
	{
		if (!ActorData.bIsSelected || !ActorData.Actor.IsValid() || !ActorData.StaticMeshComponent.IsValid())
		{
			continue;
		}

		AActor* StaticMeshActor = ActorData.Actor.Get();
		UStaticMeshComponent* SMC = ActorData.StaticMeshComponent.Get();

		// Find compatible ISM component
		UInstancedStaticMeshComponent* TargetISM = FindBestISMTarget(SMC, ISMActors);
		if (!TargetISM)
		{
			UE_LOG(LogISMMergeTool, Warning, TEXT("未找到与Actor兼容的ISM组件：%s"), *StaticMeshActor->GetName());
			continue;
		}

		// Add instance to ISM
		FTransform InstanceTransform = SMC->GetComponentTransform();
		TargetISM->Modify();
		int32 InstanceIndex = TargetISM->AddInstance(InstanceTransform);

		// Preserve material overrides
		if (Settings.bPreserveMaterialOverrides)
		{
			for (int32 MatIndex = 0; MatIndex < SMC->GetNumMaterials(); MatIndex++)
			{
				if (UMaterialInterface* OverrideMat = SMC->GetMaterial(MatIndex))
				{
					// Store material override information in custom data if possible
					// This is a simplified approach - you might want a more sophisticated system
					if (Settings.bPreserveCustomData)
					{
						TargetISM->SetCustomDataValue(InstanceIndex, MatIndex, static_cast<float>(OverrideMat->GetUniqueID()));
					}
				}
			}
		}

		MergedCount++;

		// Handle source actor
		if (Settings.bReplaceSourceActors)
		{
			// Update references
			if (ActorReferences.Contains(StaticMeshActor))
			{
				UpdateReferences(ActorReferences[StaticMeshActor], StaticMeshActor, TargetISM->GetOwner());
			}

			// Remove actor
			SafelyRemoveActor(StaticMeshActor, Settings.bDeleteSourceActors);
		}

		UE_LOG(LogISMMergeTool, Display, TEXT("已将Actor %s 合并到ISM组件 %s"),
			*StaticMeshActor->GetName(), *TargetISM->GetOwner()->GetName());
	}

	// Count unique ISM components used
	TSet<UInstancedStaticMeshComponent*> UniqueISMs;
	for (const FISMActorData& ActorData : ISMActors)
	{
		if (ActorData.bIsSelected)
		{
			for (const TWeakObjectPtr<UInstancedStaticMeshComponent>& ISM : ActorData.ISMComponents)
			{
				if (ISM.IsValid())
				{
					UniqueISMs.Add(ISM.Get());
				}
			}
		}
	}
	ISMCount = UniqueISMs.Num();

	// Refresh editor
	GEditor->RedrawLevelEditingViewports();

	// Show notification
	FNotificationInfo Info(FText::Format(
		LOCTEXT("MergeComplete", "成功将 {0} 个Actors合并到 {1} 个ISM组件"),
		FText::AsNumber(MergedCount),
		FText::AsNumber(ISMCount)));
	Info.bFireAndForget = true;
	Info.ExpireDuration = 3.0f;
	Info.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Info"));
	FSlateNotificationManager::Get().AddNotification(Info);

	UE_LOG(LogISMMergeTool, Display, TEXT("ISM合并完成。已将 %d 个Actors合并到 %d 个ISM组件"), MergedCount, ISMCount);

	return MergedCount > 0;
}

FISMCompatibilityResult UISMMergeTool::CheckISMCompatibility(UStaticMeshComponent* SourceComponent,
														   UInstancedStaticMeshComponent* TargetComponent) const
{
	if (!SourceComponent || !TargetComponent)
	{
		return FISMCompatibilityResult(false, TEXT("无效组件"));
	}

	// Check static mesh compatibility
	UStaticMesh* SourceMesh = SourceComponent->GetStaticMesh();
	UStaticMesh* TargetMesh = TargetComponent->GetStaticMesh();

	if (!SourceMesh || !TargetMesh)
	{
		return FISMCompatibilityResult(false, TEXT("缺少静态网格体"));
	}

	if (SourceMesh != TargetMesh)
	{
		return FISMCompatibilityResult(false, TEXT("静态网格体不同"));
	}

	// Check material count compatibility
	int32 SourceMaterialCount = SourceComponent->GetNumMaterials();
	int32 TargetMaterialCount = TargetComponent->GetNumMaterials();

	if (SourceMaterialCount != TargetMaterialCount)
	{
		return FISMCompatibilityResult(false, TEXT("材质槽数量不同"));
	}

	// Check collision settings
	if (SourceComponent->GetCollisionEnabled() != TargetComponent->GetCollisionEnabled())
	{
		return FISMCompatibilityResult(false, TEXT("碰撞设置不同"));
	}

	// Additional checks can be added here for:
	// - LOD settings
	// - Rendering settings
	// - Physics settings
	// etc.

	FISMCompatibilityResult Result(true);
	Result.TargetComponent = TargetComponent;
	return Result;
}

UInstancedStaticMeshComponent* UISMMergeTool::FindBestISMTarget(UStaticMeshComponent* SourceComponent,
															   const TArray<FISMActorData>& ISMActors) const
{
	for (const FISMActorData& ActorData : ISMActors)
	{
		if (!ActorData.bIsSelected || !ActorData.Actor.IsValid())
		{
			continue;
		}

		for (const TWeakObjectPtr<UInstancedStaticMeshComponent>& ISMPtr : ActorData.ISMComponents)
		{
			if (UInstancedStaticMeshComponent* ISM = ISMPtr.Get())
			{
				FISMCompatibilityResult Result = CheckISMCompatibility(SourceComponent, ISM);
				if (Result.bIsCompatible)
				{
					return ISM;
				}
			}
		}
	}

	return nullptr;
}

TArray<FISMActorData> UISMMergeTool::CollectISMActorsFromSelection() const
{
	TArray<FISMActorData> ISMActors;

	if (!GEditor)
	{
		return ISMActors;
	}

	USelection* SelectedActors = GEditor->GetSelectedActors();
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		if (AActor* Actor = Cast<AActor>(*Iter))
		{
			TArray<UInstancedStaticMeshComponent*> ISMComponents;
			Actor->GetComponents<UInstancedStaticMeshComponent>(ISMComponents);

			if (ISMComponents.Num() > 0)
			{
				ISMActors.Add(FISMActorData(Actor));
			}
		}
	}

	return ISMActors;
}

TArray<FStaticMeshActorData> UISMMergeTool::CollectStaticMeshActorsFromSelection() const
{
	TArray<FStaticMeshActorData> StaticMeshActors;

	if (!GEditor)
	{
		return StaticMeshActors;
	}

	USelection* SelectedActors = GEditor->GetSelectedActors();
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		if (AActor* Actor = Cast<AActor>(*Iter))
		{
			// Check if this actor has a static mesh component but no ISM components
			UStaticMeshComponent* SMC = Actor->FindComponentByClass<UStaticMeshComponent>();
			TArray<UInstancedStaticMeshComponent*> ISMComponents;
			Actor->GetComponents<UInstancedStaticMeshComponent>(ISMComponents);

			if (SMC && ISMComponents.Num() == 0)
			{
				StaticMeshActors.Add(FStaticMeshActorData(Actor));
			}
		}
	}

	return StaticMeshActors;
}

void UISMMergeTool::SafelyRemoveActor(AActor* Actor, bool bDeleteActor)
{
	if (!Actor)
	{
		return;
	}

	UE_LOG(LogISMMergeTool, Display, TEXT("移除Actor：%s （删除：%s）"),
		*Actor->GetName(), bDeleteActor ? TEXT("是") : TEXT("否"));

	// Mark actor for modification
	Actor->Modify();

	// Get and modify all components
	TArray<UActorComponent*> Components = Actor->GetComponents().Array();
	for (UActorComponent* Component : Components)
	{
		if (Component)
		{
			Component->Modify();

			// Clear component delegates - OnComponentDestroyed API may vary in UE5.6
			// Component->OnComponentDestroyed.Clear();

			// Clear material references for mesh components
			if (UMeshComponent* MeshComp = Cast<UMeshComponent>(Component))
			{
				int32 NumMaterials = MeshComp->GetNumMaterials();
				for (int32 i = 0; i < NumMaterials; i++)
				{
					MeshComp->SetMaterial(i, nullptr);
				}
			}
		}
	}

	if (bDeleteActor)
	{
		// Clean up references before deletion
		CleanupActorReferences(Actor);

		// Handle OFPA-specific cleanup
		HandleOFPAActorDeletion(Actor);

		// Remove from world and destroy
		if (UWorld* World = Actor->GetWorld())
		{
			World->RemoveActor(Actor, true);
		}
		Actor->Destroy();
	}
	else
	{
		// Just hide the actor
		Actor->bIsEditorOnlyActor = true;
		Actor->SetHidden(true);
		Actor->bHiddenEd = true;
		Actor->SetIsTemporarilyHiddenInEditor(true);
	}
}

void UISMMergeTool::CleanupActorReferences(AActor* ActorToRemove)
{
	if (!ActorToRemove)
	{
		return;
	}

	UE_LOG(LogISMMergeTool, Display, TEXT("清理Actor引用：%s"), *ActorToRemove->GetName());

	// Notify asset registry
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().AssetDeleted(ActorToRemove);
}

void UISMMergeTool::UpdateReferences(const TArray<UObject*>& Referencers, AActor* OldActor, AActor* NewActor)
{
	for (UObject* Referencer : Referencers)
	{
		if (!Referencer || !IsValid(Referencer))
		{
			continue;
		}

		// Handle blueprint references - simplified implementation
		if (UBlueprint* Blueprint = Cast<UBlueprint>(Referencer))
		{
			// Mark blueprint as modified
			Blueprint->Modify();
		}

		// Additional reference updating logic can be added here
		// This is a simplified implementation
	}
}

void UISMMergeTool::CollectActorReferences(AActor* Actor, TArray<UObject*>& OutReferencers)
{
	if (!Actor)
	{
		return;
	}

	// Use reflection to find all references
	// This is a simplified implementation - a full solution would use FReferenceFinder
	OutReferencers.Reset();

	// For now, we'll implement a basic version
	// A complete implementation would use:
	// FReferenceFinder RefFinder(OutReferencers);
	// RefFinder.FindReferences(Actor);
}

void UISMMergeTool::HandleOFPAActorDeletion(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	UWorld* World = Actor->GetWorld();
	if (!World)
	{
		return;
	}

	UE_LOG(LogISMMergeTool, Display, TEXT("处理OFPA Actor删除：%s"), *Actor->GetName());

	// Mark actor package as dirty
	if (UPackage* Package = Actor->GetPackage())
	{
		Package->SetDirtyFlag(true);

		// Handle external actor packages
		if (Actor->IsPackageExternal())
		{
			Package->MarkPackageDirty();
		}
	}

	// Additional OFPA-specific cleanup can be added here when needed
}

#undef LOCTEXT_NAMESPACE