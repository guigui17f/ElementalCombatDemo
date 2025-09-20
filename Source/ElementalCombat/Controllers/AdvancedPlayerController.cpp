// Copyright 2025 guigui17f. All Rights Reserved.

#include "Controllers/AdvancedPlayerController.h"
#include "UI/ExitMenuSubsystem.h"
#include "UI/SFPSWidget.h"
#include "UI/SControlGuideWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"

AAdvancedPlayerController::AAdvancedPlayerController()
{
}

void AAdvancedPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 设置默认垂直同步为开启
	SetVSyncEnabled(true);

	// 创建FPS显示控件
	if (GEngine && GEngine->GameViewport)
	{
		FPSWidget = SNew(SFPSWidget);

		// 添加到视口的右上角
		GEngine->GameViewport->AddViewportWidgetContent(
			FPSWidget.ToSharedRef(),
			1 // ZOrder - 确保在最上层
		);
	}

	// 创建操作指引控件
	if (GEngine && GEngine->GameViewport)
	{
		ControlGuideWidget = SNew(SControlGuideWidget);

		// 添加到视口的左上角
		GEngine->GameViewport->AddViewportWidgetContent(
			ControlGuideWidget.ToSharedRef(),
			1 // ZOrder - 确保在最上层
		);
	}
}

void AAdvancedPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理FPS控件
	if (FPSWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(FPSWidget.ToSharedRef());
		FPSWidget.Reset();
	}

	// 清理操作指引控件
	if (ControlGuideWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ControlGuideWidget.ToSharedRef());
		ControlGuideWidget.Reset();
	}

	Super::EndPlay(EndPlayReason);
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

void AAdvancedPlayerController::ToggleVSync()
{
	// 切换垂直同步状态
	bVSyncEnabled = !bVSyncEnabled;
	SetVSyncEnabled(bVSyncEnabled);

	UE_LOG(LogTemp, Log, TEXT("VSync toggled: %s"), bVSyncEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void AAdvancedPlayerController::SetVSyncEnabled(bool bEnabled)
{
	bVSyncEnabled = bEnabled;

	if (GEngine)
	{
		if (bEnabled)
		{
			// 开启垂直同步 - 移除帧率限制让垂直同步生效
			GEngine->SetMaxFPS(0);
			GEngine->GetGameUserSettings()->SetVSyncEnabled(true);
		}
		else
		{
			// 关闭垂直同步 - 设置为无限制帧率
			GEngine->SetMaxFPS(0);
			GEngine->GetGameUserSettings()->SetVSyncEnabled(false);
		}

		// 应用设置
		GEngine->GetGameUserSettings()->ApplySettings(false);
	}
}