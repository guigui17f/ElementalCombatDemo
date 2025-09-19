// Copyright 2025 guigui17f. All Rights Reserved.

#include "UI/SFPSWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/Engine.h"

void SFPSWidget::Construct(const FArguments& InArgs)
{
	// 初始化帧时间数组
	FrameTimes.SetNum(SmoothingWindowSize);
	for (int32 i = 0; i < SmoothingWindowSize; ++i)
	{
		FrameTimes[i] = 1.0f / 60.0f; // 默认60FPS
	}
	FrameTimeIndex = 0;

	ChildSlot
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Top)
	.Padding(10.0f, 10.0f, 10.0f, 10.0f)
	[
		SAssignNew(FPSTextBlock, STextBlock)
		.Text(FText::FromString(TEXT("FPS: 60")))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
		.ColorAndOpacity(FLinearColor::White)
		.ShadowOffset(FVector2D(1.0f, 1.0f))
		.ShadowColorAndOpacity(FLinearColor::Black)
		.Justification(ETextJustify::Right)
	];
}

void SFPSWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// 避免除零错误
	if (InDeltaTime <= 0.0f)
	{
		return;
	}

	// 记录当前帧时间
	FrameTimes[FrameTimeIndex] = InDeltaTime;
	FrameTimeIndex = (FrameTimeIndex + 1) % SmoothingWindowSize;

	// 计算平均帧时间
	float TotalFrameTime = 0.0f;
	for (float FrameTime : FrameTimes)
	{
		TotalFrameTime += FrameTime;
	}
	float AverageFrameTime = TotalFrameTime / SmoothingWindowSize;

	// 计算FPS并裁切到整数
	int32 FPS = FMath::RoundToInt(1.0f / AverageFrameTime);

	// 更新文本显示
	if (FPSTextBlock.IsValid())
	{
		FString FPSText = FString::Printf(TEXT("FPS: %d"), FPS);
		FPSTextBlock->SetText(FText::FromString(FPSText));
	}
}