// Copyright 2025 guigui17f. All Rights Reserved.

#include "PSOLoadingWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/SizeBox.h"
#include "Styling/CoreStyle.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UPSOLoadingWidget::UPSOLoadingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 设置默认值
	CurrentProgress = 0.0f;
	TargetProgress = 0.0f;
	TotalPSOCount = 0;
}

TSharedRef<SWidget> UPSOLoadingWidget::RebuildWidget()
{
	// 创建进度条
	ProgressBar = NewObject<UProgressBar>(this);
	ProgressBar->SetPercent(0.0f);
	ProgressBar->SetFillColorAndOpacity(ProgressBarColor);

	// 创建状态文本
	StatusText = NewObject<UTextBlock>(this);
	StatusText->SetText(FText::FromString(TEXT("正在初始化着色器...")));

	// 设置状态文本字体大小
	StatusText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 20));

	// 创建计数文本
	CountText = NewObject<UTextBlock>(this);
	CountText->SetText(FText::FromString(TEXT("准备中...")));

	// 设置计数文本字体大小
	CountText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));

	// 创建主容器
	UBorder* BackgroundBorder = NewObject<UBorder>(this);
	BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, BackgroundOpacity));

	UVerticalBox* MainContainer = NewObject<UVerticalBox>(this);

	// 添加顶部间距
	USpacer* TopSpacer = NewObject<USpacer>(this);
	MainContainer->AddChildToVerticalBox(TopSpacer)->SetSize(ESlateSizeRule::Fill);

	// 添加状态文本
	MainContainer->AddChildToVerticalBox(StatusText)->SetHorizontalAlignment(HAlign_Center);

	// 添加小间距
	USpacer* SmallSpacer1 = NewObject<USpacer>(this);
	SmallSpacer1->SetSize(FVector2D(0, 60));
	MainContainer->AddChildToVerticalBox(SmallSpacer1);

	// 为进度条创建水平容器以控制宽度为屏幕的80%
	UHorizontalBox* ProgressContainer = NewObject<UHorizontalBox>(this);

	// 左侧间距 (10%)
	USpacer* LeftSpacer = NewObject<USpacer>(this);
	FSlateChildSize LeftSpacerSize;
	LeftSpacerSize.SizeRule = ESlateSizeRule::Fill;
	LeftSpacerSize.Value = 0.1f;
	ProgressContainer->AddChildToHorizontalBox(LeftSpacer)->SetSize(LeftSpacerSize);

	// 进度条包装在SizeBox中以控制高度 (80%)
	USizeBox* ProgressBarSizeBox = NewObject<USizeBox>(this);
	ProgressBarSizeBox->SetContent(ProgressBar);
	ProgressBarSizeBox->SetMinDesiredHeight(35.0f); // 设置进度条高度为35像素

	FSlateChildSize ProgressBarSize;
	ProgressBarSize.SizeRule = ESlateSizeRule::Fill;
	ProgressBarSize.Value = 0.8f;
	ProgressContainer->AddChildToHorizontalBox(ProgressBarSizeBox)->SetSize(ProgressBarSize);

	// 右侧间距 (10%)
	USpacer* RightSpacer = NewObject<USpacer>(this);
	FSlateChildSize RightSpacerSize;
	RightSpacerSize.SizeRule = ESlateSizeRule::Fill;
	RightSpacerSize.Value = 0.1f;
	ProgressContainer->AddChildToHorizontalBox(RightSpacer)->SetSize(RightSpacerSize);

	// 添加进度条容器到主容器
	MainContainer->AddChildToVerticalBox(ProgressContainer);

	// 添加小间距
	USpacer* SmallSpacer2 = NewObject<USpacer>(this);
	SmallSpacer2->SetSize(FVector2D(0, 30));
	MainContainer->AddChildToVerticalBox(SmallSpacer2);

	// 添加计数文本
	MainContainer->AddChildToVerticalBox(CountText)->SetHorizontalAlignment(HAlign_Center);

	// 添加底部间距
	USpacer* BottomSpacer = NewObject<USpacer>(this);
	MainContainer->AddChildToVerticalBox(BottomSpacer)->SetSize(ESlateSizeRule::Fill);

	// 设置边框内容
	BackgroundBorder->SetContent(MainContainer);

	return BackgroundBorder->TakeWidget();
}

void UPSOLoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化UI状态
	if (ProgressBar)
	{
		ProgressBar->SetPercent(0.0f);
		ProgressBar->SetFillColorAndOpacity(ProgressBarColor);
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("正在初始化着色器...")));
	}

	if (CountText)
	{
		CountText->SetText(FText::FromString(TEXT("准备中...")));
	}

	// 启动进度平滑更新计时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			ProgressUpdateTimer,
			this,
			&UPSOLoadingWidget::UpdateProgressSmooth,
			0.016f, // ~60 FPS
			true
		);
	}
}

void UPSOLoadingWidget::SetProgress(float Progress)
{
	TargetProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
}

void UPSOLoadingWidget::SetStatusText(const FText& InStatusText)
{
	if (StatusText)
	{
		StatusText->SetText(InStatusText);
	}
}

void UPSOLoadingWidget::SetRemainingCount(int32 Remaining, int32 Total)
{
	if (CountText)
	{
		if (Total > 0)
		{
			TotalPSOCount = Total;
			FText CountDisplayText = FText::Format(
				FText::FromString(TEXT("编译进度: {0} / {1}")),
				FText::AsNumber(Total - Remaining),
				FText::AsNumber(Total)
			);
			CountText->SetText(CountDisplayText);

			// 基于剩余数量计算进度
			float CalculatedProgress = Total > 0 ? static_cast<float>(Total - Remaining) / static_cast<float>(Total) : 1.0f;
			SetProgress(CalculatedProgress);
		}
		else
		{
			CountText->SetText(FText::FromString(TEXT("准备完成...")));
		}
	}
}

void UPSOLoadingWidget::UpdateProgressSmooth()
{
	if (!ProgressBar)
	{
		return;
	}

	// 平滑插值到目标进度
	float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
	CurrentProgress = FMath::FInterpTo(CurrentProgress, TargetProgress, DeltaTime, ProgressSmoothSpeed);

	// 更新进度条
	ProgressBar->SetPercent(CurrentProgress);

	// 如果已经接近目标进度，停止计时器
	if (FMath::Abs(CurrentProgress - TargetProgress) < 0.001f && TargetProgress >= 1.0f)
	{
		CurrentProgress = TargetProgress;
		ProgressBar->SetPercent(CurrentProgress);

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(ProgressUpdateTimer);
		}
	}
}