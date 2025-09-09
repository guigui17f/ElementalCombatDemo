// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/AI/CombatAIController.h"
#include "ElementalCombatAIController.generated.h"

class AElementalCombatEnemy;

/**
 * AI Controller for ElementalCombatEnemy characters
 * Follows the framework's pure StateTree approach - minimal AI Controller code
 * All logic is handled by StateTree tasks
 */
UCLASS()
class ELEMENTALCOMBAT_API AElementalCombatAIController : public ACombatAIController
{
	GENERATED_BODY()

public:
	AElementalCombatAIController();

protected:
	/** Called when the controller possesses a pawn */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called when the controller unpossesses a pawn */
	virtual void OnUnPossess() override;

protected:
	/** Reference to the possessed elemental combat enemy */
	UPROPERTY(BlueprintReadOnly, Category="ElementalCombat|AI")
	TObjectPtr<AElementalCombatEnemy> ElementalCombatEnemy;

public:
	/** Get the possessed elemental combat enemy */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|AI")
	AElementalCombatEnemy* GetElementalCombatEnemy() const { return ElementalCombatEnemy; }
};