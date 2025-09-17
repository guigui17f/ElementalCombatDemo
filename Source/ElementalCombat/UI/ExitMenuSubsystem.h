// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExitMenuSubsystem.generated.h"

class SExitMenuWidget;
class APlayerController;

/**
 * Subsystem for managing exit menu lifecycle
 * Automatically handles cleanup between PIE sessions
 */
UCLASS()
class ELEMENTALCOMBAT_API UExitMenuSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Subsystem lifecycle */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Show the exit menu */
	UFUNCTION(BlueprintCallable, Category = "Exit Menu")
	void ShowExitMenu(APlayerController* PlayerController);

	/** Hide the exit menu */
	UFUNCTION(BlueprintCallable, Category = "Exit Menu")
	void HideExitMenu();

	/** Check if menu is currently visible */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Exit Menu")
	bool IsMenuVisible() const { return bIsMenuVisible; }

protected:
	/** Called when user makes a choice in the menu */
	void OnMenuResult(bool bConfirmed);

	/** Handle the exit logic */
	void HandleExit();

	/** Restore game state after cancelling */
	void RestoreGameState();

private:
	/** The Slate widget for the exit menu */
	TSharedPtr<SExitMenuWidget> ExitMenuWidget;

	/** Reference to the player controller that opened the menu */
	TWeakObjectPtr<APlayerController> OwningPlayerController;

	/** Flag to track menu visibility */
	bool bIsMenuVisible;
};