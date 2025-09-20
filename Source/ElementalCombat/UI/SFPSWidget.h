// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STextBlock;
class AAdvancedPlayerController;

/**
 * 简单的FPS显示Slate控件
 * 在屏幕右上角显示当前帧率和垂直同步状态
 */
class ELEMENTALCOMBAT_API SFPSWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFPSWidget) {}
	SLATE_END_ARGS()

	/** 构造控件 */
	void Construct(const FArguments& InArgs);

	/** 每帧更新 */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** FPS文本显示控件 */
	TSharedPtr<STextBlock> FPSTextBlock;

	/** VSync状态文本显示控件 */
	TSharedPtr<STextBlock> VSyncTextBlock;

	/** 用于计算平均FPS的帧时间数组 */
	TArray<float> FrameTimes;

	/** 帧时间数组的当前索引 */
	int32 FrameTimeIndex = 0;

	/** 平滑窗口大小 */
	static constexpr int32 SmoothingWindowSize = 30;

	/** 获取当前垂直同步状态 */
	bool GetVSyncStatus() const;
};