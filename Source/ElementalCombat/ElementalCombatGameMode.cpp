// Copyright Epic Games, Inc. All Rights Reserved.

#include "ElementalCombatGameMode.h"
#include "UI/PSOLoadingWidget.h"
#include "ShaderPipelineCache.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "AIController.h"
#include "Components/StateTreeAIComponent.h"

AElementalCombatGameMode::AElementalCombatGameMode()
{
	// 设置默认值
	MinimumLoadingTime = 1.0f;
	LoadingStartTime = 0.0f;
	bMinimumTimeElapsed = false;
	bPSOCompilationComplete = false;
	InitialPSOCount = 0;
	CurrentPSORemaining = 0;
}

void AElementalCombatGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 启动PSO加载流程
	StartPSOLoading();
}

void AElementalCombatGameMode::StartPSOLoading()
{
	// 记录开始时间
	LoadingStartTime = GetWorld()->GetTimeSeconds();
	bMinimumTimeElapsed = false;
	bPSOCompilationComplete = false;

	// 暂停游戏和输入
	PauseGameplay();

	// 创建加载界面
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PSOLoadingWidget = CreateWidget<UPSOLoadingWidget>(PC, UPSOLoadingWidget::StaticClass());
		if (PSOLoadingWidget)
		{
			PSOLoadingWidget->AddToViewport(1000); // 高优先级显示
			PSOLoadingWidget->SetStatusText(FText::FromString(TEXT("正在优化着色器性能...")));
		}
	}

	// 设置PSO加载模式为Precompile模式
	FShaderPipelineCache::SetBatchMode(FShaderPipelineCache::BatchMode::Precompile);

	// 获取初始PSO数量
	InitialPSOCount = FShaderPipelineCache::NumPrecompilesRemaining();
	CurrentPSORemaining = InitialPSOCount;

	UE_LOG(LogTemp, Log, TEXT("PSO Loading Started - Initial count: %d"), InitialPSOCount);

	// 如果没有PSO需要编译，标记完成但仍需等待最小显示时间
	if (InitialPSOCount == 0)
	{
		bPSOCompilationComplete = true;
		if (PSOLoadingWidget)
		{
			PSOLoadingWidget->SetStatusText(FText::FromString(TEXT("系统准备就绪")));
			PSOLoadingWidget->SetProgress(1.0f);
		}
		UE_LOG(LogTemp, Log, TEXT("No PSO to compile, waiting for minimum display time"));
	}
	else
	{
		// 启动进度检查计时器
		GetWorldTimerManager().SetTimer(
			PSOProgressTimer,
			this,
			&AElementalCombatGameMode::CheckPSOProgress,
			0.1f, // 每0.1秒检查一次
			true
		);
	}

	// 启动最小显示时间计时器
	GetWorldTimerManager().SetTimer(
		MinDisplayTimer,
		this,
		&AElementalCombatGameMode::OnMinimumTimeElapsed,
		MinimumLoadingTime,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("Minimum display timer started for %.1f seconds"), MinimumLoadingTime);
}

void AElementalCombatGameMode::CheckPSOProgress()
{
	CurrentPSORemaining = FShaderPipelineCache::NumPrecompilesRemaining();

	// 更新UI
	if (PSOLoadingWidget && InitialPSOCount > 0)
	{
		PSOLoadingWidget->SetRemainingCount(CurrentPSORemaining, InitialPSOCount);

		// 更新状态文本
		if (CurrentPSORemaining > 0)
		{
			FText StatusText = FText::Format(
				FText::FromString(TEXT("正在编译着色器... ({0} 剩余)")),
				FText::AsNumber(CurrentPSORemaining)
			);
			PSOLoadingWidget->SetStatusText(StatusText);
		}
		else
		{
			PSOLoadingWidget->SetStatusText(FText::FromString(TEXT("编译完成，准备启动...")));
		}
	}

	// 检查是否完成
	if (CurrentPSORemaining == 0 && !bPSOCompilationComplete)
	{
		bPSOCompilationComplete = true;
		UE_LOG(LogTemp, Log, TEXT("PSO Compilation completed"));

		// 如果最小时间也已经过了，立即完成加载
		if (bMinimumTimeElapsed)
		{
			CompletePSOLoading();
		}
	}
}

void AElementalCombatGameMode::OnMinimumTimeElapsed()
{
	bMinimumTimeElapsed = true;
	UE_LOG(LogTemp, Log, TEXT("Minimum display time elapsed, PSO complete: %s"),
		   bPSOCompilationComplete ? TEXT("true") : TEXT("false"));

	// 如果PSO编译也完成了，完成加载
	if (bPSOCompilationComplete)
	{
		UE_LOG(LogTemp, Log, TEXT("Both conditions met, completing PSO loading"));
		CompletePSOLoading();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Still waiting for PSO compilation to complete"));
	}
}

void AElementalCombatGameMode::CompletePSOLoading()
{
	UE_LOG(LogTemp, Log, TEXT("Completing PSO loading"));

	// 停止计时器
	GetWorldTimerManager().ClearTimer(PSOProgressTimer);
	GetWorldTimerManager().ClearTimer(MinDisplayTimer);

	// 切换PSO模式回Background模式
	FShaderPipelineCache::SetBatchMode(FShaderPipelineCache::BatchMode::Background);

	// 隐藏加载界面
	if (PSOLoadingWidget)
	{
		PSOLoadingWidget->SetStatusText(FText::FromString(TEXT("启动完成")));
		PSOLoadingWidget->SetProgress(1.0f);

		// 延迟一点时间再移除UI，让用户看到完成状态
		FTimerHandle RemoveUITimer;
		GetWorldTimerManager().SetTimer(
			RemoveUITimer,
			[this]()
			{
				if (PSOLoadingWidget)
				{
					PSOLoadingWidget->RemoveFromParent();
					PSOLoadingWidget = nullptr;
				}
				// 恢复游戏和输入
				ResumeGameplay();
			},
			0.5f,
			false
		);
	}
	else
	{
		// 如果没有UI，直接恢复游戏
		ResumeGameplay();
	}
}

void AElementalCombatGameMode::PauseGameplay()
{
	// 设置暂停标志
	bIsGameplayPausedForPSO = true;

	// 设置输入模式为仅UI（不显示鼠标）
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PC->SetInputMode(InputMode);
		PC->DisableInput(PC);
	}

	// 暂停所有AI控制器的逻辑
	for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
	{
		AAIController* AIController = *It;
		if (AIController && AIController->GetPawn())
		{
			// 停止AI移动
			AIController->StopMovement();

			// 暂停StateTree组件（如果有）
			if (UStateTreeAIComponent* StateTreeComp = AIController->FindComponentByClass<UStateTreeAIComponent>())
			{
				StateTreeComp->StopLogic(TEXT("PSO Loading"));
			}

			UE_LOG(LogTemp, VeryVerbose, TEXT("Paused AI Controller: %s"), *AIController->GetName());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Input disabled for PSO loading"));
}

void AElementalCombatGameMode::ResumeGameplay()
{
	// 清除暂停标志
	bIsGameplayPausedForPSO = false;

	// 恢复游戏输入模式
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->EnableInput(PC);
	}

	// 恢复所有AI控制器的逻辑
	for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
	{
		AAIController* AIController = *It;
		if (AIController && AIController->GetPawn())
		{
			// 恢复StateTree组件（如果有）
			if (UStateTreeAIComponent* StateTreeComp = AIController->FindComponentByClass<UStateTreeAIComponent>())
			{
				StateTreeComp->RestartLogic();
			}

			UE_LOG(LogTemp, VeryVerbose, TEXT("Resumed AI Controller: %s"), *AIController->GetName());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Input and AI enabled after PSO loading"));
}
