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

	// 重置激活标志，为重新激活做准备
	bHasBeenActivated = false;

	UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 已重置。生成数量: %d"),
		*GetName(), SpawnCount);
}

void AReusableEnemySpawner::SpawnerDepleted()
{
	// 调用父类逻辑激活后续Actor
	Super::SpawnerDepleted();

	// 如果允许重新激活，重置激活标志以便下次激活
	if (CanBeReactivated())
	{
		bHasBeenActivated = false;
		UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 队列耗尽但可以重新激活"), *GetName());
	}
	else
	{
		UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 永久耗尽"), *GetName());
	}
}

void AReusableEnemySpawner::ActivateInteraction(AActor* ActivationInstigator)
{
	// 检查是否可以重新激活（除非是第一次激活）
	if (bHasBeenActivated && !CanBeReactivated())
	{
		UE_LOG(LogElementalAI, Warning, TEXT("生成器 %s 无法重新激活"), *GetName());
		return;
	}

	// 如果已经激活且当前正在生成（定时器活跃），忽略重复激活
	if (bHasBeenActivated && GetWorld()->GetTimerManager().IsTimerActive(SpawnTimer))
	{
		UE_LOG(LogElementalAI, Warning, TEXT("生成器 %s 已经在活跃状态"), *GetName());
		return;
	}

	// 检查是否需要重置（队列已耗尽）
	if (SpawnCount <= 0)
	{
		ResetSpawner();
		// 只有在重新激活时才增加计数器
		if (bHasBeenActivated)
		{
			CurrentReactivations++;
		}
	}

	// 设置激活标志并开始生成敌人
	bHasBeenActivated = true;
	SpawnEnemy();

	if (CurrentReactivations > 0)
	{
		UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 已重新激活 (次数: %d/%d)"),
			*GetName(), CurrentReactivations, MaxReactivations);
	}
	else
	{
		UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 首次激活"), *GetName());
	}
}

void AReusableEnemySpawner::DeactivateInteraction(AActor* ActivationInstigator)
{
	// 停止当前的生成过程
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	
	UE_LOG(LogElementalAI, Log, TEXT("生成器 %s 已停用"), *GetName());
}