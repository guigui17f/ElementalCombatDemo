#include "ReusableEnemySpawner.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ElementalCombat.h"

AReusableEnemySpawner::AReusableEnemySpawner()
{
	// 默认允许重新激活
	bCanBeReactivated = true;
	MaxReactivations = -1; // 无限制
}

void AReusableEnemySpawner::BeginPlay()
{
	// 保存原始配置
	OriginalSpawnCount = SpawnCount;
	
	// 调用父类BeginPlay
	Super::BeginPlay();
}

bool AReusableEnemySpawner::CanBeReactivated() const
{
	// 检查是否允许重新激活
	if (!bCanBeReactivated)
	{
		return false;
	}

	// 检查激活次数限制
	if (MaxReactivations >= 0 && CurrentReactivations >= MaxReactivations)
	{
		return false;
	}

	return true;
}

void AReusableEnemySpawner::ResetSpawner()
{
	// 重置生成计数
	SpawnCount = OriginalSpawnCount;
	
	// 清理现有定时器
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	
	// 重置激活标志（如果允许重新激活）
	if (bCanBeReactivated)
	{
		bHasBeenActivated = false;
	}

	UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 已重置。生成数量: %d"), 
		*GetName(), SpawnCount);
}

void AReusableEnemySpawner::SpawnerDepleted()
{
	// 调用父类逻辑激活后续Actor
	Super::SpawnerDepleted();
	
	// 如果允许重新激活，准备下次使用
	if (CanBeReactivated())
	{
		UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 队列耗尽但可以重新激活"), *GetName());
	}
	else
	{
		UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 永久耗尽"), *GetName());
	}
}

void AReusableEnemySpawner::ActivateInteraction(AActor* ActivationInstigator)
{
	// 如果是第一次激活，使用父类逻辑
	if (!bHasBeenActivated)
	{
		Super::ActivateInteraction(ActivationInstigator);
		return;
	}

	// 检查是否可以重新激活
	if (!CanBeReactivated())
	{
		UE_LOG(LogElementalAI, Warning, TEXT("生成器 %s 无法重新激活"), *GetName());
		return;
	}

	// 检查是否需要重置（队列已耗尽）
	if (SpawnCount <= 0)
	{
		ResetSpawner();
		CurrentReactivations++;
	}

	// 如果当前没有活动的定时器，开始生成敌人
	if (!GetWorld()->GetTimerManager().IsTimerActive(SpawnTimer))
	{
		bHasBeenActivated = true;
		SpawnEnemy();
		
		UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 已重新激活 (次数: %d/%d)"), 
			*GetName(), CurrentReactivations, MaxReactivations);
	}
}

void AReusableEnemySpawner::DeactivateInteraction(AActor* ActivationInstigator)
{
	// 停止当前的生成过程
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	
	UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 已停用"), *GetName());
}