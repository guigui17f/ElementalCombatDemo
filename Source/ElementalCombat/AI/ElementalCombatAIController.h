// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/AI/CombatAIController.h"
#include "AI/Utility/UtilityAITypes.h"
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

	/** 当前AI的Utility配置 */
	UPROPERTY(BlueprintReadOnly, Category="ElementalCombat|AI")
	FUtilityProfile CurrentAIProfile;

public:
	/** Get the possessed elemental combat enemy */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|AI")
	AElementalCombatEnemy* GetElementalCombatEnemy() const { return ElementalCombatEnemy; }

	/** 获取当前AI的Utility配置（供StateTree访问） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|AI")
	const FUtilityProfile& GetCurrentAIProfile() const { return CurrentAIProfile; }

#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	/** 为测试场景设置AI配置（仅在测试或编辑器构建中可用） */
	void SetAIProfileForTest(const FUtilityProfile& TestProfile);
#endif

private:
	/** 创建默认测试配置（当GameInstance不可用时的后备方案） */
	static FUtilityProfile CreateDefaultTestProfile();
};