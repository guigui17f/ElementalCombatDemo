// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "ElementalCombatGameMode.generated.h"

class UPSOLoadingWidget;

/**
 *  GameMode for ElementalCombat with PSO loading screen support
 */
UCLASS(abstract)
class AElementalCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	AElementalCombatGameMode();

protected:
	/** Called when the game starts */
	virtual void BeginPlay() override;


	/** Minimum time to display loading screen (in seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ElementalCombat", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MinimumLoadingTime;

private:
	/** Current PSO loading widget instance */
	UPROPERTY()
	TObjectPtr<UPSOLoadingWidget> PSOLoadingWidget;

	/** Timer handle for PSO progress checking */
	FTimerHandle PSOProgressTimer;

	/** Timer handle for minimum display time */
	FTimerHandle MinDisplayTimer;

	/** Time when loading started */
	float LoadingStartTime = 0.0f;

	/** Whether minimum display time has elapsed */
	bool bMinimumTimeElapsed = false;

	/** Whether PSO compilation is complete */
	bool bPSOCompilationComplete = false;

	/** Initial number of PSOs to compile */
	uint32 InitialPSOCount = 0;

	/** Current number of PSOs remaining */
	uint32 CurrentPSORemaining = 0;

public:
	/** Whether gameplay is paused for PSO loading */
	UPROPERTY(BlueprintReadOnly)
	bool bIsGameplayPausedForPSO = false;

	/** Check if gameplay is currently paused */
	UFUNCTION(BlueprintCallable)
	bool IsGameplayPausedForPSO() const { return bIsGameplayPausedForPSO; }

private:
	/**
	 * Start the PSO loading process
	 */
	void StartPSOLoading();

	/**
	 * Check PSO compilation progress
	 */
	void CheckPSOProgress();

	/**
	 * Complete the PSO loading process
	 */
	void CompletePSOLoading();

	/**
	 * Called when minimum display time elapses
	 */
	void OnMinimumTimeElapsed();

	/**
	 * Pause game and disable input
	 */
	void PauseGameplay();

	/**
	 * Resume game and enable input
	 */
	void ResumeGameplay();
};



