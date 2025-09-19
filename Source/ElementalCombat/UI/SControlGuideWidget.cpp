// Copyright 2025 guigui17f. All Rights Reserved.

#include "UI/SControlGuideWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/AppStyle.h"
#include "Engine/Engine.h"

void SControlGuideWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	.Padding(20.0f, 20.0f, 20.0f, 20.0f)
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.4f)) // 半透明黑色背景
		.Padding(FMargin(15.0f)) // 内容与边框的间距
		[
			SAssignNew(GuideContainer, SVerticalBox)

		// ESC - 退出游戏
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			CreateGuideItem(TEXT("WASD"), TEXT("移动角色"))
		]

		// WASD - 移动角色
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			CreateGuideItem(TEXT("鼠标左键"), TEXT("近战普通攻击"))
		]

		// 鼠标左键 - 近战普通攻击
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			CreateGuideItem(TEXT("鼠标右键"), TEXT("远程元素攻击"))
		]

		// 鼠标右键 - 远程元素攻击
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			CreateGuideItem(TEXT("1/2/3/4/5"), TEXT("切换元素属性"))
		]

		// 1/2/3/4/5 - 切换元素属性
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 0.0f)
		[
			CreateGuideItem(TEXT("ESC"), TEXT("退出游戏"))
		]
		]
	];
}

TSharedRef<SWidget> SControlGuideWidget::CreateGuideItem(const FString& KeyText, const FString& DescriptionText) const
{
	return SNew(SHorizontalBox)

		// 按键部分
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(KeyText))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
			.ColorAndOpacity(FLinearColor::Yellow)  // 按键用黄色突出显示
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor::Black)
		]

		// 分隔符 " - "
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("-")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
			.ColorAndOpacity(FLinearColor::White)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor::Black)
		]

		// 说明部分
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(FText::FromString(DescriptionText))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
			.ColorAndOpacity(FLinearColor::White)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor::Black)
		];
}