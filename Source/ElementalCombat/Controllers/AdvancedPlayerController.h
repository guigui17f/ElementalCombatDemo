// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/CombatPlayerController.h"
#include "AdvancedPlayerController.generated.h"

class SFPSWidget;

/**
 * Advanced Player Controller that extends CombatPlayerController
 * Adds ESC key handling for exit menu functionality and includes respawn functionality
 */
UCLASS(BlueprintType, Blueprintable)
class ELEMENTALCOMBAT_API AAdvancedPlayerController : public ACombatPlayerController
{
	GENERATED_BODY()

public:
	AAdvancedPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** Show the exit menu - called by Character input */
	UFUNCTION(BlueprintCallable, Category = "Exit Menu")
	void ShowExitMenu();

private:
	/** FPS显示控件 */
	TSharedPtr<SFPSWidget> FPSWidget;
};