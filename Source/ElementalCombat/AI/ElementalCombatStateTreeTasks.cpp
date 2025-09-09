// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalCombatStateTreeTasks.h"
#include "ElementalCombatEnemy.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"

// Ranged Projectile Attack Task Implementation
EStateTreeRunStatus FStateTreeRangedProjectileAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.EnemyCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("FStateTreeRangedProjectileAttackTask: EnemyCharacter is null"));
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("FStateTreeRangedProjectileAttackTask: Executing ranged attack for %s"), *InstanceData.EnemyCharacter->GetName());
	
	// Execute ranged attack
	InstanceData.EnemyCharacter->DoAIRangedAttack();
	
	return EStateTreeRunStatus::Running;
}

void FStateTreeRangedProjectileAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// Clean up if needed
	UE_LOG(LogTemp, Log, TEXT("FStateTreeRangedProjectileAttackTask: Exiting ranged attack state"));
}

#if WITH_EDITOR
FText FStateTreeRangedProjectileAttackTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return NSLOCTEXT("ElementalCombat", "RangedProjectileAttackTask", "Execute Ranged Projectile Attack");
}
#endif

// Select Attack Type Task Implementation
EStateTreeRunStatus FStateTreeSelectAttackTypeTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.EnemyCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("FStateTreeSelectAttackTypeTask: EnemyCharacter is null"));
		return EStateTreeRunStatus::Failed;
	}

	// Use distance from GetPlayerInfo task (should be bound in StateTree)
	float Distance = InstanceData.DistanceToTarget;
	
	if (Distance <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("FStateTreeSelectAttackTypeTask: Invalid distance %.2f - check GetPlayerInfo binding"), Distance);
		return EStateTreeRunStatus::Failed;
	}

	// Decide attack type based on distance
	EAIAttackType AttackType = InstanceData.EnemyCharacter->DecideAttackType(Distance);
	
	// Use UE official Output category - direct assignment with reference (as per UE documentation)
	FStateTreeSelectAttackTypeInstanceData& MutableInstanceData = Context.GetInstanceData(*this);
	MutableInstanceData.SelectedAttackType = AttackType;
	
	const TCHAR* AttackTypeName = (AttackType == EAIAttackType::Melee) ? TEXT("Melee") : 
								  (AttackType == EAIAttackType::Ranged) ? TEXT("Ranged") : TEXT("None");
	
	UE_LOG(LogTemp, Log, TEXT("FStateTreeSelectAttackTypeTask: Selected %s attack (type %d) for distance %.2f - output set"), 
		   AttackTypeName, (int32)AttackType, Distance);
	
	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FStateTreeSelectAttackTypeTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return NSLOCTEXT("ElementalCombat", "SelectAttackTypeTask", "Select Attack Type Based on Distance");
}
#endif

// Maintain Preferred Range Task Implementation
EStateTreeRunStatus FStateTreeMaintainPreferredRangeTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.EnemyCharacter || !InstanceData.AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("FStateTreeMaintainPreferredRangeTask: Missing EnemyCharacter or AIController"));
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.TargetPlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("FStateTreeMaintainPreferredRangeTask: No TargetPlayerCharacter - check GetPlayerInfo binding"));
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("FStateTreeMaintainPreferredRangeTask: Starting preferred range maintenance for target %s"), 
		   *InstanceData.TargetPlayerCharacter->GetName());
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeMaintainPreferredRangeTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.EnemyCharacter || !InstanceData.AIController || !InstanceData.TargetPlayerCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Use distance from GetPlayerInfo task binding
	float DistanceToTarget = InstanceData.DistanceToTarget;
	
	if (DistanceToTarget <= 0.0f)
	{
		// Fallback calculation if binding is not working
		DistanceToTarget = FVector::Dist(InstanceData.EnemyCharacter->GetActorLocation(), InstanceData.TargetPlayerCharacter->GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("FStateTreeMaintainPreferredRangeTask: Using fallback distance calculation"));
	}
	
	// Check if in preferred range
	bool bWasInRange = InstanceData.InPreferredRange;
	bool bCurrentlyInRange = InstanceData.EnemyCharacter->IsInPreferredRange(DistanceToTarget);
	
	// Use UE official Output category - direct assignment with reference (as per UE documentation)
	FStateTreeMaintainPreferredRangeInstanceData& MutableInstanceData = Context.GetInstanceData(*this);
	MutableInstanceData.InPreferredRange = bCurrentlyInRange;
	
	// Only log when status changes to avoid spam
	if (bWasInRange != bCurrentlyInRange)
	{
		UE_LOG(LogTemp, Log, TEXT("FStateTreeMaintainPreferredRangeTask: Range status output changed to %s"), 
			   bCurrentlyInRange ? TEXT("InRange") : TEXT("OutOfRange"));
	}
	
	// Log range status change
	if (bWasInRange != bCurrentlyInRange)
	{
		UE_LOG(LogTemp, Log, TEXT("FStateTreeMaintainPreferredRangeTask: Range status changed to %s (Distance: %.2f)"), 
			   bCurrentlyInRange ? TEXT("InRange") : TEXT("OutOfRange"), DistanceToTarget);
	}

	// Move towards preferred range if not in range
	if (!bCurrentlyInRange)
	{
		// Use target location from GetPlayerInfo task or current player location
		FVector TargetLocation = InstanceData.TargetPlayerLocation.IsZero() ? 
			InstanceData.TargetPlayerCharacter->GetActorLocation() : InstanceData.TargetPlayerLocation;
			
		// Calculate preferred position
		FVector DirectionToTarget = (TargetLocation - InstanceData.EnemyCharacter->GetActorLocation()).GetSafeNormal();
		FVector PreferredPosition = TargetLocation - (DirectionToTarget * 500.0f); // PreferredAttackRange
		
		InstanceData.AIController->MoveToLocation(PreferredPosition, 50.0f);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeMaintainPreferredRangeTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	// Stop movement
	if (InstanceData.AIController)
	{
		InstanceData.AIController->StopMovement();
	}
	
	UE_LOG(LogTemp, Log, TEXT("FStateTreeMaintainPreferredRangeTask: Exiting range maintenance state"));
}

#if WITH_EDITOR
FText FStateTreeMaintainPreferredRangeTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return NSLOCTEXT("ElementalCombat", "MaintainPreferredRangeTask", "Maintain Preferred Attack Range");
}
#endif