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
		UE_LOG(LogTemp, Warning, TEXT("%s: DecideAttackType called with invalid distance %.2f"), *GetName(), DistanceToTarget);
		return EAIAttackType::None;
	}

	EAIAttackType AttackType = EAIAttackType::None;

	if (DistanceToTarget <= MeleeAttackRange)
	{
		AttackType = EAIAttackType::Melee;
		UE_LOG(LogTemp, Log, TEXT("%s: Selected Melee attack (Distance: %.2f <= %.2f)"), *GetName(), DistanceToTarget, MeleeAttackRange);
	}
	else if (DistanceToTarget <= RangedAttackRange)
	{
		AttackType = EAIAttackType::Ranged;
		UE_LOG(LogTemp, Log, TEXT("%s: Selected Ranged attack (Distance: %.2f <= %.2f)"), *GetName(), DistanceToTarget, RangedAttackRange);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s: No attack type selected (Distance: %.2f > %.2f)"), *GetName(), DistanceToTarget, RangedAttackRange);
	}

	return AttackType;
}

bool AElementalCombatEnemy::IsInPreferredRange(float DistanceToTarget) const
{
	// 在偏好距离的100单位容差范围内
	bool bInRange = FMath::Abs(DistanceToTarget - PreferredAttackRange) < 100.0f;
	UE_LOG(LogTemp, Verbose, TEXT("%s: IsInPreferredRange - Distance: %.2f, Preferred: %.2f, InRange: %s"), 
		   *GetName(), DistanceToTarget, PreferredAttackRange, bInRange ? TEXT("Yes") : TEXT("No"));
	return bInRange;
}

void AElementalCombatEnemy::DoAIRangedAttack()
{
	UE_LOG(LogTemp, Log, TEXT("%s: DoAIRangedAttack called"), *GetName());

	// 如果正在攻击，忽略
	if (bIsAttacking)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Already attacking, ignoring ranged attack request"), *GetName());
		return;
	}

	// 设置攻击状态
	bIsAttacking = true;
	CurrentAttackType = EAIAttackType::Ranged;
	UE_LOG(LogTemp, Log, TEXT("%s: Started ranged attack sequence"), *GetName());

	// 播放远程攻击动画
	if (RangedAttackMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Playing ranged attack montage"), *GetName());
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		
		// 绑定动画结束回调
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
		{
			bIsAttacking = false;
			CurrentAttackType = EAIAttackType::None;
			UE_LOG(LogTemp, Log, TEXT("%s: Ranged attack animation completed"), *GetName());
		});

		AnimInstance->Montage_Play(RangedAttackMontage);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, RangedAttackMontage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No ranged attack montage configured, launching projectile directly"), *GetName());
		
		// 如果没有动画，直接发射投掷物
		LaunchProjectile();
		
		// 延迟重置攻击状态
		FTimerHandle AttackResetTimer;
		GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
		{
			UE_LOG(LogTemp, Log, TEXT("%s: Ranged attack completed (no animation)"), *GetName());
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
	UE_LOG(LogTemp, Log, TEXT("%s: LaunchProjectile called"), *GetName());

	// 检查是否有投掷物类
	if (!AIProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: AIProjectileClass not set, cannot launch projectile"), *GetName());
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
		UE_LOG(LogTemp, Log, TEXT("%s: Successfully spawned projectile %s"), *GetName(), *Projectile->GetName());

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
		UE_LOG(LogTemp, Error, TEXT("%s: Failed to spawn projectile"), *GetName());
	}
}

float AElementalCombatEnemy::GetDistanceToTarget() const
{
	// Note: This method is now mainly used for compatibility
	// StateTree tasks should use GetPlayerInfo task data instead of calling this method
	
	// Try to find player character as fallback
	if (UWorld* World = GetWorld())
	{
		if (ACharacter* PlayerCharacter = Cast<ACharacter>(World->GetFirstPlayerController()->GetPawn()))
		{
			float Distance = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
			UE_LOG(LogTemp, Verbose, TEXT("%s: Fallback distance calculation to player: %.2f"), *GetName(), Distance);
			return Distance;
		}
	}

	// If no player found, return max distance
	UE_LOG(LogTemp, Warning, TEXT("%s: No target available for distance calculation"), *GetName());
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