// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PSOLoadingWidget.generated.h"

/**
 * PSO (Pipeline State Object) 加载界面控件
 * 在游戏启动时显示着色器预编译进度
 */
UCLASS(BlueprintType, Blueprintable)
class ELEMENTALCOMBAT_API UPSOLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPSOLoadingWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	/**
	 * 更新进度条
	 * @param Progress 进度值 (0.0 - 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|UI")
	void SetProgress(float Progress);

	/**
	 * 设置加载状态文本
	 * @param StatusText 要显示的状态文本
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|UI")
	void SetStatusText(const FText& StatusText);

	/**
	 * 设置加载剩余数量
	 * @param Remaining 剩余待编译的PSO数量
	 * @param Total 总PSO数量
	 */
	UFUNCTION(BlueprintCallable, Category = "ElementalCombat|UI")
	void SetRemainingCount(int32 Remaining, int32 Total);

protected:
	/** 进度条控件 */
	UPROPERTY()
	TObjectPtr<UProgressBar> ProgressBar;

	/** 状态文本控件 */
	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText;

	/** 计数文本控件 */
	UPROPERTY()
	TObjectPtr<UTextBlock> CountText;

	/** 背景遮罩的不透明度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|UI")
	float BackgroundOpacity = 0.95f;

	/** 进度条颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|UI")
	FLinearColor ProgressBarColor = FLinearColor(0.0f, 0.6f, 1.0f, 1.0f);

private:
	/** 当前显示的进度值，用于平滑过渡 */
	float CurrentProgress = 0.0f;

	/** 目标进度值 */
	float TargetProgress = 0.0f;

	/** 总PSO数量，用于计算进度 */
	int32 TotalPSOCount = 0;

	/** 进度平滑插值速度 */
	UPROPERTY(EditAnywhere, Category = "ElementalCombat|UI", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ProgressSmoothSpeed = 5.0f;

	/** 用于平滑进度更新的计时器 */
	FTimerHandle ProgressUpdateTimer;

	/**
	 * 平滑更新进度条
	 */
	void UpdateProgressSmooth();
};