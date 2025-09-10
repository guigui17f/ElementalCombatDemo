// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"
#include "ElementalCombatEnemy.h"
#include "ElementalCombatStateTreeTasks.generated.h"

class AElementalCombatEnemy;
class AAIController;

/**
 * Instance data struct for elemental combat StateTree tasks
 */
USTRUCT()
struct FStateTreeElementalCombatInstanceData
{
	GENERATED_BODY()

	/** Elemental combat enemy character that will perform actions */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AElementalCombatEnemy> EnemyCharacter;
};

// Add POD macro for proper StateTree binding
STATETREE_POD_INSTANCEDATA(FStateTreeElementalCombatInstanceData);

/**
 * StateTree task to perform ranged projectile attack
 */
USTRUCT(meta=(DisplayName="Ranged Projectile Attack", Category="ElementalCombat|AI"))
struct ELEMENTALCOMBAT_API FStateTreeRangedProjectileAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* Ensure we're using the correct instance data struct */
	using FInstanceDataType = FStateTreeElementalCombatInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FStateTreeRangedProjectileAttackTask() = default;

	/** Runs when the owning state is entered */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/** Runs when the owning state is ended */
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

/**
 * Instance data for attack type selection task
 * Uses the same data flow as GetPlayerInfo task
 */
USTRUCT()
struct FStateTreeSelectAttackTypeInstanceData
{
	GENERATED_BODY()

	/** Elemental combat enemy character */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AElementalCombatEnemy> EnemyCharacter;

	/** Distance to target from GetPlayerInfo task */
	UPROPERTY(EditAnywhere, Category = Input)
	float DistanceToTarget = 0.0f;

	/** Target player character from GetPlayerInfo task */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<ACharacter> TargetPlayerCharacter;

	/** Output value for selected attack type - follows UE official Output category standard */
	UPROPERTY(EditAnywhere, Category = Output,
			  meta = (DisplayName = "Selected Attack Type",
			  		  ToolTip = "Selected attack type based on distance analysis"))
	EAIAttackType SelectedAttackType = EAIAttackType::None;
};

// Add POD macro for proper StateTree binding
STATETREE_POD_INSTANCEDATA(FStateTreeSelectAttackTypeInstanceData);

/**
 * StateTree task to select attack type based on distance
 */
USTRUCT(meta=(DisplayName="Select Attack Type", Category="ElementalCombat|AI"))
struct ELEMENTALCOMBAT_API FStateTreeSelectAttackTypeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* Ensure we're using the correct instance data struct */
	using FInstanceDataType = FStateTreeSelectAttackTypeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FStateTreeSelectAttackTypeTask() = default;

	/** Runs when the owning state is entered */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

/**
 * Instance data for maintain preferred range task
 * Uses GetPlayerInfo task output
 */
USTRUCT()
struct FStateTreeMaintainPreferredRangeInstanceData
{
	GENERATED_BODY()

	/** Elemental combat enemy character */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AElementalCombatEnemy> EnemyCharacter;

	/** AI Controller for movement */
	UPROPERTY(EditAnywhere, Category = Context)  
	TObjectPtr<AAIController> AIController;

	/** Target player character from GetPlayerInfo task */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<ACharacter> TargetPlayerCharacter;

	/** Target location from GetPlayerInfo task */
	UPROPERTY(EditAnywhere, Category = Input)
	FVector TargetPlayerLocation = FVector::ZeroVector;

	/** Distance to target from GetPlayerInfo task */
	UPROPERTY(EditAnywhere, Category = Input)
	float DistanceToTarget = 0.0f;

	/** Output value for range status - follows UE official Output category standard */
	UPROPERTY(EditAnywhere, Category = Output,
			  meta = (DisplayName = "In Preferred Range", 
			  		  ToolTip = "Whether the character is currently in preferred attack range"))
	bool InPreferredRange = false;
};

// Add POD macro for proper StateTree binding
STATETREE_POD_INSTANCEDATA(FStateTreeMaintainPreferredRangeInstanceData);

/**
 * StateTree task to maintain preferred attack range
 */
USTRUCT(meta=(DisplayName="Maintain Preferred Range", Category="ElementalCombat|AI"))
struct ELEMENTALCOMBAT_API FStateTreeMaintainPreferredRangeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* Ensure we're using the correct instance data struct */
	using FInstanceDataType = FStateTreeMaintainPreferredRangeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FStateTreeMaintainPreferredRangeTask() = default;

	/** Runs when the owning state is entered */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/** Runs while the owning state is active */
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	/** Runs when the owning state is ended */
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};