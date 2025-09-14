// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalCombatEnemy.h"
#include "Projectiles/CombatProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Variant_Combat/UI/CombatLifeBar.h"

AElementalCombatEnemy::AElementalCombatEnemy()
{
	// 设置默认值
	MeleeAttackRange = 200.0f;
	RangedAttackRange = 1000.0f;
	PreferredAttackRange = 500.0f;
	ProjectileSocketName = TEXT("hand_r");
}

void AElementalCombatEnemy::BeginPlay()
{
	Super::BeginPlay();
}

EAIAttackType AElementalCombatEnemy::DecideAttackType(float DistanceToTarget) const
{
	if (DistanceToTarget <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: DecideAttackType被调用时传入无效距离%.2f"), *GetName(), DistanceToTarget);
		return EAIAttackType::None;
	}

	// 简单返回当前配置的攻击类型，让StateTree控制决策逻辑
	UE_LOG(LogTemp, Log, TEXT("%s: 使用已配置的攻击类型 %d （距离: %.2f）"),
		   *GetName(), (int32)CurrentAttackType, DistanceToTarget);

	return CurrentAttackType;
}

bool AElementalCombatEnemy::IsInPreferredRange(float DistanceToTarget) const
{
	// 让StateTree的Utility系统完全控制范围判断，这里始终返回true
	UE_LOG(LogTemp, Verbose, TEXT("%s: 是否在偏好范围内 - 距离: %.2f （StateTree控制范围逻辑）"),
		   *GetName(), DistanceToTarget);
	return true;
}

void AElementalCombatEnemy::DoAIRangedAttack()
{
	UE_LOG(LogTemp, Log, TEXT("%s: DoAIRangedAttack被调用"), *GetName());

	// 如果正在攻击，忽略
	if (bIsAttacking)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: 已在攻击，忽略远程攻击请求"), *GetName());
		return;
	}

	// 设置攻击状态
	bIsAttacking = true;
	CurrentAttackType = EAIAttackType::Ranged;
	UE_LOG(LogTemp, Log, TEXT("%s: 开始远程攻击序列"), *GetName());

	// 播放远程攻击动画
	if (RangedAttackMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Log, TEXT("%s: 播放远程攻击动画序列"), *GetName());
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		
		// 绑定动画结束回调
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
		{
			bIsAttacking = false;
			CurrentAttackType = EAIAttackType::None;
			UE_LOG(LogTemp, Log, TEXT("%s: 远程攻击动画完成"), *GetName());
		});

		AnimInstance->Montage_Play(RangedAttackMontage);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, RangedAttackMontage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: 未配置远程攻击动画序列，直接发射投射物"), *GetName());
		
		// 如果没有动画，直接发射投掷物
		LaunchProjectile();
		
		// 延迟重置攻击状态
		FTimerHandle AttackResetTimer;
		GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
		{
			UE_LOG(LogTemp, Log, TEXT("%s: 远程攻击完成（无动画）"), *GetName());
			bIsAttacking = false;
			CurrentAttackType = EAIAttackType::None;
			
			if (OnAttackCompleted.IsBound())
			{
				OnAttackCompleted.Execute();
			}
		}, 1.0f, false);
	}

	// 触发蓝图事件
	OnRangedAttackStarted();
}

void AElementalCombatEnemy::LaunchProjectile()
{
	UE_LOG(LogTemp, Log, TEXT("%s: LaunchProjectile被调用"), *GetName());

	// 检查是否有投掷物类
	if (!AIProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: AI投射物类未设置，无法发射投射物"), *GetName());
		return;
	}

	// 获取发射位置和方向
	FVector SpawnLocation;
	FRotator SpawnRotation;
	GetProjectileLaunchParams(SpawnLocation, SpawnRotation);

	// 设置生成参数
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 生成投掷物
	ACombatProjectile* Projectile = GetWorld()->SpawnActor<ACombatProjectile>(
		AIProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (Projectile)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: 成功生成投射物 %s"), *GetName(), *Projectile->GetName());

		// AI使用固定的投掷物属性（可以根据难度调整）
		float SpeedMultiplier = 1.0f;
		float DamageMultiplier = 1.0f;

		// 可以根据AI类型或难度设置不同的倍率
		Projectile->SetProjectileProperties(SpeedMultiplier, DamageMultiplier);

		// 触发蓝图事件
		OnProjectileLaunched(Projectile);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s: 生成投射物失败"), *GetName());
	}
}

float AElementalCombatEnemy::GetDistanceToTarget() const
{
	// 注意：这个方法现在主要用于兼容性
	// StateTree任务应该使用GetPlayerInfo任务数据而不是调用这个方法
	
	// 尝试找到玩家角色作为备用方案
	if (UWorld* World = GetWorld())
	{
		if (ACharacter* PlayerCharacter = Cast<ACharacter>(World->GetFirstPlayerController()->GetPawn()))
		{
			float Distance = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
			UE_LOG(LogTemp, Verbose, TEXT("%s: 备用计算到玩家的距离：%.2f"), *GetName(), Distance);
			return Distance;
		}
	}

	// 如果没有找到玩家，返回最大距离
	UE_LOG(LogTemp, Warning, TEXT("%s: 没有目标可用于距离计算"), *GetName());
	return TNumericLimits<float>::Max();
}

float AElementalCombatEnemy::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, 
                                       AController* EventInstigator, AActor* DamageCauser)
{
	// 只处理活着的角色
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// 减少当前HP
	CurrentHP -= Damage;

	// HP耗尽？
	if (CurrentHP <= 0.0f)
	{
		// 死亡（保留ragdoll）
		HandleDeath();
	}
	else
	{
		// 更新生命条
		if (LifeBarWidget)
		{
			LifeBarWidget->SetLifePercentage(CurrentHP / MaxHP);
		}
		
		// 不启用ragdoll物理
	}

	return Damage;
}

void AElementalCombatEnemy::GetProjectileLaunchParams(FVector& OutLocation, FRotator& OutRotation) const
{
	// 获取发射位置（从Socket）
	if (GetMesh() && GetMesh()->DoesSocketExist(ProjectileSocketName))
	{
		OutLocation = GetMesh()->GetSocketLocation(ProjectileSocketName);
	}
	else
	{
		// 如果没有Socket，使用角色位置加偏移
		OutLocation = GetActorLocation() + FVector(0, 0, 80.0f);
	}

	// 获取发射方向（朝向目标）
	FVector TargetLocation = GetActorLocation() + GetActorForwardVector() * 1000.0f; // 默认前方

	// 尝试获取玩家角色位置（简化版，不依赖黑板）
	if (UWorld* World = GetWorld())
	{
		if (ACharacter* PlayerCharacter = Cast<ACharacter>(World->GetFirstPlayerController()->GetPawn()))
		{
			TargetLocation = PlayerCharacter->GetActorLocation();
			// 瞄准目标中心
			TargetLocation.Z += 50.0f;
		}
	}

	// 计算朝向目标的旋转
	FVector Direction = (TargetLocation - OutLocation).GetSafeNormal();
	OutRotation = Direction.Rotation();
}