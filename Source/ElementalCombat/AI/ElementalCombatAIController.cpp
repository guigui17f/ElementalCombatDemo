// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalCombatAIController.h"
#include "ElementalCombatEnemy.h"

AElementalCombatAIController::AElementalCombatAIController()
{
	// Minimal constructor - all logic handled by StateTree
}

void AElementalCombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Cache reference to the elemental combat enemy
	ElementalCombatEnemy = Cast<AElementalCombatEnemy>(InPawn);
	
	if (ElementalCombatEnemy)
	{
		UE_LOG(LogTemp, Log, TEXT("ElementalCombatAIController: Possessed ElementalCombatEnemy %s"), *ElementalCombatEnemy->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementalCombatAIController: Possessed pawn is not an ElementalCombatEnemy"));
	}
}

void AElementalCombatAIController::OnUnPossess()
{
	// Clear references
	ElementalCombatEnemy = nullptr;

	Super::OnUnPossess();
}