// Copyright 2025 guigui17f. All Rights Reserved.

#include "UI/SElementalHUD.h"
#include "Combat/Elemental/ElementalComponent.h"
#include "Combat/Elemental/ElementalCalculator.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/AppStyle.h"
#include "Engine/Engine.h"

void SElementalHUD::Construct(const FArguments& InArgs)
{
	ElementalComponentRef = InArgs._ElementalComponent;

	// 注意：委托绑定将由包装控件处理，因为Slate控件不支持UObject委托

	ChildSlot
	[
		SNew(SBox)
		.Padding(FMargin(20.0f, 20.0f)) // 增加与屏幕边缘的距离
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.MaxDesiredWidth(600.0f) // 增加最大宽度避免换行
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f)) // 半透明黑色背景
			.Padding(FMargin(15.0f)) // 内容与边框的间距
			[
				SNew(SVerticalBox)

			// 关系链条显示 - 使用水平布局分别显示每个元素
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 8))
			[
				SNew(SHorizontalBox)

				// 克制当前元素的元素
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0, 0, 5, 0))
				[
					SAssignNew(CounterElementText, STextBlock)
					.Text(this, &SElementalHUD::GetCounterElementText)
					.ColorAndOpacity(this, &SElementalHUD::GetCounterElementColor)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				]

				// 箭头符号 ">"
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(5, 0, 5, 0))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT(">")))
					.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				]

				// 当前元素（放大显示）
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(5, 0, 5, 0))
				[
					SAssignNew(CurrentElementText, STextBlock)
					.Text(this, &SElementalHUD::GetCurrentElementText)
					.ColorAndOpacity(this, &SElementalHUD::GetCurrentElementColor)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20)) // 当前元素使用粗体20号字体
				]

				// 箭头符号 ">"
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(5, 0, 5, 0))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT(">")))
					.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				]

				// 当前元素克制的元素
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(5, 0, 0, 0))
				[
					SAssignNew(CounteredElementText, STextBlock)
					.Text(this, &SElementalHUD::GetCounteredElementText)
					.ColorAndOpacity(this, &SElementalHUD::GetCounteredElementColor)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				]
			]

			// 效果描述显示
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(DescriptionText, STextBlock)
				.Text(this, &SElementalHUD::GetCurrentElementDescription)
				.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 0.9f))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				.Justification(ETextJustify::Left)
			]
		]
		] // SBorder 结束
	]; // SBox 结束

	// 初始更新
	UpdateDisplay();
}

void SElementalHUD::UpdateDisplay()
{
	// 强制刷新文本块
	if (CounterElementText.IsValid())
	{
		CounterElementText->Invalidate(EInvalidateWidget::Layout);
	}
	if (CurrentElementText.IsValid())
	{
		CurrentElementText->Invalidate(EInvalidateWidget::Layout);
	}
	if (CounteredElementText.IsValid())
	{
		CounteredElementText->Invalidate(EInvalidateWidget::Layout);
	}
	if (DescriptionText.IsValid())
	{
		DescriptionText->Invalidate(EInvalidateWidget::Layout);
	}
}

FText SElementalHUD::GetElementDisplayName(EElementalType Element) const
{
	switch (Element)
	{
	case EElementalType::Metal:
		return FText::FromString(TEXT("金"));
	case EElementalType::Wood:
		return FText::FromString(TEXT("木"));
	case EElementalType::Water:
		return FText::FromString(TEXT("水"));
	case EElementalType::Fire:
		return FText::FromString(TEXT("火"));
	case EElementalType::Earth:
		return FText::FromString(TEXT("土"));
	default:
		return FText::FromString(TEXT("无"));
	}
}

FLinearColor SElementalHUD::GetElementDisplayColor(EElementalType Element) const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FLinearColor::White;
	}

	const FElementalEffectData* ElementData = ElementalComponentRef->GetElementEffectDataPtr(Element);
	if (ElementData)
	{
		return ElementData->ElementColor;
	}

	// 如果没有可用数据，使用默认颜色
	switch (Element)
	{
	case EElementalType::Metal:
		return FLinearColor::White;
	case EElementalType::Wood:
		return FLinearColor::Green;
	case EElementalType::Water:
		return FLinearColor::Blue;
	case EElementalType::Fire:
		return FLinearColor::Red;
	case EElementalType::Earth:
		return FLinearColor(0.8f, 0.6f, 0.2f); // 棕色调
	default:
		return FLinearColor::Gray;
	}
}

FText SElementalHUD::GetCurrentElementDescription() const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FText::GetEmpty();
	}

	EElementalType CurrentElement = ElementalComponentRef->GetCurrentElement();
	const FElementalEffectData* ElementData = ElementalComponentRef->GetElementEffectDataPtr(CurrentElement);

	if (ElementData && !ElementData->EffectDescription.IsEmpty())
	{
		return ElementData->EffectDescription;
	}

	// 如果没有配置数据，使用默认描述
	switch (CurrentElement)
	{
	case EElementalType::Metal:
		return FText::FromString(TEXT("金元素：提升攻击伤害"));
	case EElementalType::Wood:
		return FText::FromString(TEXT("木元素：攻击时吸取生命值"));
	case EElementalType::Water:
		return FText::FromString(TEXT("水元素：攻击减速敌人"));
	case EElementalType::Fire:
		return FText::FromString(TEXT("火元素：造成持续燃烧伤害"));
	case EElementalType::Earth:
		return FText::FromString(TEXT("土元素：提升防御能力"));
	default:
		return FText::FromString(TEXT("无元素"));
	}
}

FText SElementalHUD::GetRelationshipChainText() const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FText::GetEmpty();
	}

	EElementalType CurrentElement = ElementalComponentRef->GetCurrentElement();

	// 获取关系元素
	EElementalType CounterElement = UElementalCalculator::GetElementThatCounters(CurrentElement);
	EElementalType CounteredElement = UElementalCalculator::GetElementCounteredBy(CurrentElement);

	// 构建关系链条：克制者 > 当前元素 > 被克制者
	FString ChainString;

	if (CounterElement != EElementalType::None)
	{
		ChainString += GetElementDisplayName(CounterElement).ToString();
		ChainString += TEXT(" > ");
	}

	ChainString += GetElementDisplayName(CurrentElement).ToString();

	if (CounteredElement != EElementalType::None)
	{
		ChainString += TEXT(" > ");
		ChainString += GetElementDisplayName(CounteredElement).ToString();
	}

	return FText::FromString(ChainString);
}

FText SElementalHUD::GetCounterElementText() const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FText::GetEmpty();
	}

	EElementalType CurrentElement = ElementalComponentRef->GetCurrentElement();
	EElementalType CounterElement = UElementalCalculator::GetElementThatCounters(CurrentElement);

	if (CounterElement != EElementalType::None)
	{
		return GetElementDisplayName(CounterElement);
	}

	return FText::GetEmpty();
}

FText SElementalHUD::GetCurrentElementText() const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FText::GetEmpty();
	}

	EElementalType CurrentElement = ElementalComponentRef->GetCurrentElement();
	return GetElementDisplayName(CurrentElement);
}

FText SElementalHUD::GetCounteredElementText() const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FText::GetEmpty();
	}

	EElementalType CurrentElement = ElementalComponentRef->GetCurrentElement();
	EElementalType CounteredElement = UElementalCalculator::GetElementCounteredBy(CurrentElement);

	if (CounteredElement != EElementalType::None)
	{
		return GetElementDisplayName(CounteredElement);
	}

	return FText::GetEmpty();
}

FSlateColor SElementalHUD::GetCounterElementColor() const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FSlateColor(FLinearColor::White);
	}

	EElementalType CurrentElement = ElementalComponentRef->GetCurrentElement();
	EElementalType CounterElement = UElementalCalculator::GetElementThatCounters(CurrentElement);

	if (CounterElement != EElementalType::None)
	{
		return FSlateColor(GetElementDisplayColor(CounterElement));
	}

	return FSlateColor(FLinearColor::Gray);
}

FSlateColor SElementalHUD::GetCurrentElementColor() const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FSlateColor(FLinearColor::White);
	}

	EElementalType CurrentElement = ElementalComponentRef->GetCurrentElement();
	return FSlateColor(GetElementDisplayColor(CurrentElement));
}

FSlateColor SElementalHUD::GetCounteredElementColor() const
{
	if (!ElementalComponentRef.IsValid())
	{
		return FSlateColor(FLinearColor::White);
	}

	EElementalType CurrentElement = ElementalComponentRef->GetCurrentElement();
	EElementalType CounteredElement = UElementalCalculator::GetElementCounteredBy(CurrentElement);

	if (CounteredElement != EElementalType::None)
	{
		return FSlateColor(GetElementDisplayColor(CounteredElement));
	}

	return FSlateColor(FLinearColor::Gray);
}

void SElementalHUD::OnElementChanged(EElementalType NewElement)
{
	// 更新显示
	UpdateDisplay();
}