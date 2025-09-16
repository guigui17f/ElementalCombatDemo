// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Combat/Elemental/ElementalTypes.h"
#include "ElementalHUDWidget.generated.h"

class SElementalHUD;
class UElementalComponent;

/**
 * 基于Slate的元素HUD的包装控件
 * 提供蓝图集成功能并管理原生Slate控件
 */
UCLASS(BlueprintType, Blueprintable)
class ELEMENTALCOMBAT_API UElementalHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UElementalHUDWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * 设置要观察的元素组件
	 * @param InElementalComponent 要监听元素变化的组件
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|UI")
	void SetElementalComponent(UElementalComponent* InElementalComponent);

	/**
	 * 更新HUD显示
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|UI")
	void UpdateDisplay();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * 当元素组件的元素发生变化时调用
	 */
	UFUNCTION()
	void OnElementChanged(EElementalType NewElement);

private:
	/** 原生Slate控件 */
	TSharedPtr<SElementalHUD> SlateWidget;

	/** 我们正在观察的元素组件的引用 */
	UPROPERTY()
	TObjectPtr<UElementalComponent> ElementalComponent;
};