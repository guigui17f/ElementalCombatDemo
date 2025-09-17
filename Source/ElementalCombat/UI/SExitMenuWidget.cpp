// Copyright 2025 guigui17f. All Rights Reserved.

#include "UI/SExitMenuWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"
#include "Input/Events.h"

const FVector2D SExitMenuWidget::DialogSize = FVector2D(500.0f, 250.0f);

void SExitMenuWidget::Construct(const FArguments& InArgs)
{
	OnResultDelegate = InArgs._OnResult;

	ChildSlot
	[
		// Semi-transparent background overlay
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f)) // Semi-transparent black
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(0)
			[
				// Main dialog box
				SNew(SBox)
				.WidthOverride(DialogSize.X)
				.HeightOverride(DialogSize.Y)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 0.75f)) // Semi-transparent dark gray background
					.Padding(FMargin(20.0f))
					[
						SNew(SVerticalBox)

						// Title section
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0, 0, 0, 20))
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("退出确认")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
							.ColorAndOpacity(FLinearColor::White)
							.Justification(ETextJustify::Center)
						]

						// Message section
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(FMargin(0, 0, 0, 20))
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("确定要退出游戏吗？")))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
							.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
							.Justification(ETextJustify::Center)
						]

						// Buttons section
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)

							// Confirm button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(FMargin(0, 0, 15, 0))
							[
								SNew(SBox)
								.WidthOverride(120.0f)
								.HeightOverride(40.0f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.1f, 0.7f, 0.1f, 0.6f)) // Semi-transparent green background
									.Padding(FMargin(2.0f))
									[
										SNew(SButton)
										.ButtonStyle(FCoreStyle::Get(), "NoBorder")
										.ButtonColorAndOpacity(FLinearColor::Transparent)
										.OnClicked(this, &SExitMenuWidget::OnConfirmClicked)
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
											.Text(FText::FromString(TEXT("确定")))
											.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
											.ColorAndOpacity(FLinearColor::White)
										]
									]
								]
							]

							// Cancel button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(FMargin(15, 0, 0, 0))
							[
								SNew(SBox)
								.WidthOverride(120.0f)
								.HeightOverride(40.0f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 0.6f)) // Semi-transparent gray background
									.Padding(FMargin(2.0f))
									[
										SNew(SButton)
										.ButtonStyle(FCoreStyle::Get(), "NoBorder")
										.ButtonColorAndOpacity(FLinearColor::Transparent)
										.OnClicked(this, &SExitMenuWidget::OnCancelClicked)
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
											.Text(FText::FromString(TEXT("取消")))
											.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
											.ColorAndOpacity(FLinearColor::White)
										]
									]
								]
							]
						]
					]
				]
			]
		]
	];
}

FVector2D SExitMenuWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return DialogSize * LayoutScaleMultiplier;
}

FReply SExitMenuWidget::OnConfirmClicked()
{
	if (OnResultDelegate.IsBound())
	{
		OnResultDelegate.ExecuteIfBound(true);
	}
	return FReply::Handled();
}

FReply SExitMenuWidget::OnCancelClicked()
{
	if (OnResultDelegate.IsBound())
	{
		OnResultDelegate.ExecuteIfBound(false);
	}
	return FReply::Handled();
}

FReply SExitMenuWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Handle ESC key to cancel
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnCancelClicked();
		return FReply::Handled();
	}

	// Handle Enter key to confirm
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		OnConfirmClicked();
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}