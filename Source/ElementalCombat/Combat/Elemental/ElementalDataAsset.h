// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ElementalTypes.h"
#include "ElementalDataAsset.generated.h"

/**
 * 元素配置数据资产
 * 用于在编辑器中配置元素效果和相克关系
 */
UCLASS(BlueprintType)
class ELEMENTALCOMBAT_API UElementalDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UElementalDataAsset();

	/**
	 * 获取指定元素的效果数据
	 * @param Element 元素类型
	 * @param OutEffectData 输出的效果数据
	 * @return true如果找到数据
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	bool GetElementEffectData(EElementalType Element, FElementalEffectData& OutEffectData) const;

	/**
	 * 获取指定元素的相克关系
	 * @param Element 元素类型
	 * @param OutRelationship 输出的相克关系
	 * @return true如果找到关系
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	bool GetElementRelationship(EElementalType Element, FElementalRelationship& OutRelationship) const;

	/**
	 * 获取指定元素的效果数据（C++用）
	 * @param Element 元素类型
	 * @return 效果数据指针，如果不存在返回nullptr
	 */
	const FElementalEffectData* GetElementEffectDataPtr(EElementalType Element) const;

	/**
	 * 获取指定元素的相克关系（C++用）
	 * @param Element 元素类型
	 * @return 相克关系指针，如果不存在返回nullptr
	 */
	const FElementalRelationship* GetElementRelationshipPtr(EElementalType Element) const;

	/**
	 * 检查是否有指定元素的配置
	 * @param Element 元素类型
	 * @return true如果有配置
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	bool HasElementConfiguration(EElementalType Element) const;

	/**
	 * 验证数据配置的完整性
	 * @return 验证结果和错误信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Elemental")
	bool ValidateData(FString& OutErrorMessage) const;

	/**
	 * 获取所有配置的元素类型
	 * @return 元素类型数组
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	TArray<EElementalType> GetConfiguredElements() const;

	/**
	 * 复制元素数据到组件
	 * @param ElementComponent 目标元素组件
	 */
	UFUNCTION(BlueprintCallable, Category = "Elemental")
	void CopyDataToComponent(class UElementalComponent* ElementComponent) const;

#if WITH_EDITOR
	// 编辑器中验证数据
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

protected:
	// 元素效果数据配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element Effects", meta = (TitleProperty = "Element"))
	TArray<FElementalEffectData> ElementEffects;

	// 元素相克关系配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element Relationships", meta = (TitleProperty = "Element"))
	TArray<FElementalRelationship> ElementRelationships;

	// 默认元素效果数据
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defaults")
	FElementalEffectData DefaultEffectData;

	// 构建元素映射缓存（派生类可访问）
	void BuildElementMaps();

private:

	// 元素效果映射缓存（运行时用）
	UPROPERTY(Transient)
	TMap<EElementalType, FElementalEffectData> ElementEffectMap;

	// 元素关系映射缓存（运行时用）
	UPROPERTY(Transient)
	TMap<EElementalType, FElementalRelationship> ElementRelationshipMap;

	// 缓存是否已构建
	bool bCacheBuilt;
};