// Copyright 2025 guigui17f. All Rights Reserved.

#include "Controllers/AdvancedPlayerController.h"
#include "UI/ExitMenuSubsystem.h"
#include "Engine/Engine.h"

AAdvancedPlayerController::AAdvancedPlayerController()
{
}

void AAdvancedPlayerController::ShowExitMenu()
{
	// Get the exit menu subsystem
	if (UExitMenuSubsystem* ExitMenuSubsystem = GetGameInstance()->GetSubsystem<UExitMenuSubsystem>())
	{
		// Show the exit menu if it's not already visible
		if (!ExitMenuSubsystem->IsMenuVisible())
		{
			ExitMenuSubsystem->ShowExitMenu(this);
		}
	}
}