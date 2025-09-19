// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;

/**
 * 操作指引Slate控件
 * 在屏幕左上角显示游戏操作说明
 */
class ELEMENTALCOMBAT_API SControlGuideWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SControlGuideWidget) {}
	SLATE_END_ARGS()

	/** 构造控件 */
	void Construct(const FArguments& InArgs);

private:
	/** 主要的垂直布局容器 */
	TSharedPtr<SVerticalBox> GuideContainer;

	/** 创建单行操作说明 */
	TSharedRef<SWidget> CreateGuideItem(const FString& KeyText, const FString& DescriptionText) const;
};