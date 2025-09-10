// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ElementalTypes.h"
#include "ElementalComponent.generated.h"

class ACombatProjectile;

// 元素变更委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElementChanged, EElementalType, NewElement);

/**
 * 元素组件
 * 负责管理Actor的当前元素状态、元素切换、效果数据存储等功能
 */
UCLASS(ClassGroup=(ElementalCombat), meta=(BlueprintSpawnableComponent))
class ELEMENTALCOMBAT_API UElementalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UElementalComponent();

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * 切换到指定元素
	 * @param NewElement 新的元素类型
	 */
	UFUNCTION(BlueprintCallable, Category = "Elemental")
	void SwitchElement(EElementalType NewElement);

	/**
	 * 获取当前元素
	 * @return 当前元素类型
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	EElementalType GetCurrentElement() const { return CurrentElement; }

	/**
	 * 设置元素效果数据
	 * @param Element 元素类型
	 * @param EffectData 效果数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Elemental")
	void SetElementEffectData(EElementalType Element, const FElementalEffectData& EffectData);

	/**
	 * 获取元素效果数据
	 * @param Element 元素类型
	 * @param OutEffectData 输出的效果数据
	 * @return true如果找到数据
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	bool GetElementEffectData(EElementalType Element, FElementalEffectData& OutEffectData) const;

	/**
	 * 获取元素效果数据（C++用）
	 * @param Element 元素类型
	 * @return 效果数据指针，如果不存在返回nullptr
	 */
	const FElementalEffectData* GetElementEffectDataPtr(EElementalType Element) const;

	/**
	 * 获取当前元素的投掷物类
	 * @return 投掷物类，如果没有设置返回nullptr
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	UClass* GetCurrentProjectileClass() const;

	/**
	 * 检查是否有指定元素的数据
	 * @param Element 元素类型
	 * @return true如果有该元素的数据
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	bool HasElementData(EElementalType Element) const;

public:
	// 元素变更委托
	UPROPERTY(BlueprintAssignable, Category = "Elemental")
	FOnElementChanged OnElementChanged;

protected:
	// 当前元素
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elemental")
	EElementalType CurrentElement;

	// 元素效果数据映射
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elemental")
	TMap<EElementalType, FElementalEffectData> ElementEffectDataMap;

private:
	// 触发元素变更委托
	void BroadcastElementChanged(EElementalType NewElement);
};