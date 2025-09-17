// Copyright 2025 guigui17f. All Rights Reserved.

#include "UI/ExitMenuSubsystem.h"
#include "UI/SExitMenuWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void UExitMenuSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsMenuVisible = false;
}

void UExitMenuSubsystem::Deinitialize()
{
	// Clean up any remaining widgets
	HideExitMenu();
	Super::Deinitialize();
}

void UExitMenuSubsystem::ShowExitMenu(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (bIsMenuVisible)
	{
		return;
	}

	OwningPlayerController = PlayerController;
	bIsMenuVisible = true;

	// Pause the game
	if (UWorld* World = PlayerController->GetWorld())
	{
		World->GetFirstPlayerController()->SetPause(true);
	}

	// Create the exit menu widget
	ExitMenuWidget = SNew(SExitMenuWidget)
		.OnResult(FOnExitMenuResult::CreateUObject(this, &UExitMenuSubsystem::OnMenuResult));

	if (!ExitMenuWidget.IsValid())
	{
		return;
	}

	// Set input mode to UI only and show mouse cursor
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ExitMenuWidget);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);

	// Add the widget as viewport overlay
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(ExitMenuWidget.ToSharedRef(), 1000); // High Z-order
	}
}

void UExitMenuSubsystem::HideExitMenu()
{
	if (!bIsMenuVisible)
	{
		return;
	}

	// Remove the viewport widget
	if (ExitMenuWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ExitMenuWidget.ToSharedRef());
	}

	// Clean up references
	ExitMenuWidget.Reset();

	RestoreGameState();

	bIsMenuVisible = false;
}

void UExitMenuSubsystem::OnMenuResult(bool bConfirmed)
{
	if (bConfirmed)
	{
		HandleExit();
	}
	else
	{
		HideExitMenu();
	}
}

void UExitMenuSubsystem::HandleExit()
{
	// Quit the application - works in both editor PIE and standalone builds
	if (OwningPlayerController.IsValid())
	{
		APlayerController* PlayerController = OwningPlayerController.Get();
		if (IsValid(PlayerController))
		{
			if (UWorld* World = PlayerController->GetWorld())
			{
				UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, false);
			}
		}
	}
}

void UExitMenuSubsystem::RestoreGameState()
{
	if (!OwningPlayerController.IsValid())
	{
		return;
	}

	// Get a strong reference for safe access
	APlayerController* PlayerController = OwningPlayerController.Get();
	if (!IsValid(PlayerController))
	{
		OwningPlayerController.Reset();
		return;
	}

	// Restore input mode to game only and hide mouse cursor first
	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(false);

	// Unpause the game last (after other systems are ready)
	if (UWorld* World = PlayerController->GetWorld())
	{
		World->GetFirstPlayerController()->SetPause(false);
	}

	// Clear the reference
	OwningPlayerController.Reset();
}