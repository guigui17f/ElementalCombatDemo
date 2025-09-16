// Copyright 2025 guigui17f. All Rights Reserved.

#include "UI/ElementalHUDWidget.h"
#include "UI/SElementalHUD.h"
#include "Combat/Elemental/ElementalComponent.h"

UElementalHUDWidget::UElementalHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ElementalComponent(nullptr)
{
}

void UElementalHUDWidget::SetElementalComponent(UElementalComponent* InElementalComponent)
{
	// 如果存在，从之前的组件解绑
	if (ElementalComponent && ElementalComponent->OnElementChanged.IsBound())
	{
		ElementalComponent->OnElementChanged.RemoveDynamic(this, &UElementalHUDWidget::OnElementChanged);
	}

	ElementalComponent = InElementalComponent;

	// 绑定到新组件
	if (ElementalComponent)
	{
		ElementalComponent->OnElementChanged.AddDynamic(this, &UElementalHUDWidget::OnElementChanged);
	}

	// 如果Slate控件存在，更新它
	if (SlateWidget.IsValid())
	{
		SlateWidget->UpdateDisplay();
	}
}

void UElementalHUDWidget::UpdateDisplay()
{
	if (SlateWidget.IsValid())
	{
		SlateWidget->UpdateDisplay();
	}
}

TSharedRef<SWidget> UElementalHUDWidget::RebuildWidget()
{
	SlateWidget = SNew(SElementalHUD)
		.ElementalComponent(ElementalComponent);

	return SlateWidget.ToSharedRef();
}

void UElementalHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 如果需要，进行额外的初始化
	if (SlateWidget.IsValid())
	{
		SlateWidget->UpdateDisplay();
	}
}

void UElementalHUDWidget::NativeDestruct()
{
	// 从元素组件委托中解绑
	if (ElementalComponent && ElementalComponent->OnElementChanged.IsBound())
	{
		ElementalComponent->OnElementChanged.RemoveDynamic(this, &UElementalHUDWidget::OnElementChanged);
	}

	// 清理Slate控件引用
	SlateWidget.Reset();

	Super::NativeDestruct();
}

void UElementalHUDWidget::OnElementChanged(EElementalType NewElement)
{
	// 更新Slate控件显示
	UpdateDisplay();
}