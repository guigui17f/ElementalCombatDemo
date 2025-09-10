#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/AI/CombatEnemySpawner.h"
#include "ReusableEnemySpawner.generated.h"

/**
 * 扩展版本的敌人生成器，支持重复激活
 * 基于CombatEnemySpawner，添加重新激活功能
 */
UCLASS(BlueprintType, Blueprintable)
class ELEMENTALCOMBAT_API AReusableEnemySpawner : public ACombatEnemySpawner
{
	GENERATED_BODY()

protected:
	/** 是否允许重新激活 */
	UPROPERTY(EditAnywhere, Category="ElementalCombat|AI")
	bool bCanBeReactivated = true;

	/** 重新激活次数限制（-1表示无限制） */
	UPROPERTY(EditAnywhere, Category="ElementalCombat|AI", meta = (ClampMin = -1, ClampMax = 100))
	int32 MaxReactivations = -1;

	/** 保存初始生成数量，用于重置 */
	UPROPERTY(VisibleAnywhere, Category="ElementalCombat|AI")
	int32 OriginalSpawnCount = 1;
	
	/** 当前已重新激活次数 */
	UPROPERTY(VisibleAnywhere, Category="ElementalCombat|AI")
	int32 CurrentReactivations = 0;

public:
	/** 构造函数 */
	AReusableEnemySpawner();

	/** 重置生成器状态，准备重新激活 */
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|AI")
	void ResetSpawner();

	/** 检查是否可以重新激活 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|AI")
	bool CanBeReactivated() const;

protected:
	/** 重写BeginPlay以保存初始配置 */
	virtual void BeginPlay() override;

	/** 重写队列耗尽逻辑 */
	virtual void SpawnerDepleted() override;

public:
	// ~begin ICombatActivatable interface

	/** 重写激活逻辑，支持重新激活 */
	virtual void ActivateInteraction(AActor* ActivationInstigator) override;

	/** 实现去激活逻辑 */
	virtual void DeactivateInteraction(AActor* ActivationInstigator) override;

	// ~end ICombatActivatable interface
};