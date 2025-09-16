// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Combat/Elemental/ElementalTypes.h"

class UElementalComponent;

/**
 * 用于显示元素关系和效果的Slate控件
 * 显示元素克制链条：克制者 > 当前元素 > 被克制者
 * 显示当前元素的效果描述
 */
class ELEMENTALCOMBAT_API SElementalHUD : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SElementalHUD)
		: _ElementalComponent(nullptr)
		{}

		/** 要观察的元素组件 */
		SLATE_ARGUMENT(UElementalComponent*, ElementalComponent)

	SLATE_END_ARGS()

	/** 使用参数构造此控件 */
	void Construct(const FArguments& InArgs);

	/** 使用当前元素数据更新HUD */
	void UpdateDisplay();

protected:
	/** 我们正在观察的元素组件的引用 */
	TWeakObjectPtr<UElementalComponent> ElementalComponentRef;

	/** 将元素类型转换为显示名称 */
	FText GetElementDisplayName(EElementalType Element) const;

	/** 从元素数据中获取元素显示颜色 */
	FLinearColor GetElementDisplayColor(EElementalType Element) const;

	/** 获取当前元素的效果描述 */
	FText GetCurrentElementDescription() const;

	/** 获取关系链条文本（例如："水 > 火 > 金"） */
	FText GetRelationshipChainText() const;

	/** 获取克制当前元素的元素名称 */
	FText GetCounterElementText() const;

	/** 获取当前元素名称 */
	FText GetCurrentElementText() const;

	/** 获取当前元素克制的元素名称 */
	FText GetCounteredElementText() const;

	/** 获取克制当前元素的元素颜色 */
	FSlateColor GetCounterElementColor() const;

	/** 获取当前元素颜色 */
	FSlateColor GetCurrentElementColor() const;

	/** 获取当前元素克制的元素颜色 */
	FSlateColor GetCounteredElementColor() const;

	/** 元素改变时的委托处理 */
	void OnElementChanged(EElementalType NewElement);

private:
	/** 克制当前元素的元素文本块 */
	TSharedPtr<class STextBlock> CounterElementText;

	/** 当前元素文本块 */
	TSharedPtr<class STextBlock> CurrentElementText;

	/** 当前元素克制的元素文本块 */
	TSharedPtr<class STextBlock> CounteredElementText;

	/** 效果描述的文本块 */
	TSharedPtr<class STextBlock> DescriptionText;
};