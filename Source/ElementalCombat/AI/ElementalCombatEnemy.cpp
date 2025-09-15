// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalCombatEnemy.h"
#include "Projectiles/CombatProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Variant_Combat/UI/CombatLifeBar.h"
#include "Combat/Elemental/ElementalComponent.h"
#include "Combat/Elemental/ElementalDataAsset.h"
#include "Materials/MaterialInstanceDynamic.h"

AElementalCombatEnemy::AElementalCombatEnemy()
{
	// 设置默认值
	MeleeAttackRange = 200.0f;
	RangedAttackRange = 1000.0f;
	PreferredAttackRange = 500.0f;
	ProjectileSocketName = TEXT("hand_r");

	// 创建元素组件
	ElementalComponent = CreateDefaultSubobject<UElementalComponent>(TEXT("ElementalComponent"));
}

void AElementalCombatEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 初始化元素 - 从配置中随机选择
	if (ElementalComponent)
	{
		// 获取所有配置的元素类型
		if (UElementalDataAsset* DataAsset = ElementalComponent->GetElementalDataAsset())
		{
			TArray<EElementalType> ConfiguredElements = DataAsset->GetConfiguredElements();

			// 过滤掉None元素
			ConfiguredElements.RemoveAll([](EElementalType Element) {
				return Element == EElementalType::None;
			});

			// 如果有配置的元素，随机选择一个
			if (ConfiguredElements.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, ConfiguredElements.Num() - 1);
				EElementalType SelectedElement = ConfiguredElements[RandomIndex];
				ElementalComponent->SwitchElement(SelectedElement);

				// 应用元素颜色到材质
				const FElementalEffectData* ElementData = ElementalComponent->GetElementEffectDataPtr(SelectedElement);
				if (ElementData)
				{
					UpdateMaterialColors(ElementData->ElementColor);
				}

				UE_LOG(LogTemp, Log, TEXT("%s: 随机选择了 %d 元素"),
					*GetName(), (int32)SelectedElement);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s: 没有找到可用的配置元素，保持None状态"), *GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: 无法获取ElementalDataAsset"), *GetName());
		}
	}
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

	// 优先使用元素特定的投掷物类（匹配玩家角色的行为）
	UClass* ProjectileClassToUse = nullptr;
	if (ElementalComponent)
	{
		EElementalType CurrentElement = ElementalComponent->GetCurrentElement();
		ProjectileClassToUse = ElementalComponent->GetCurrentProjectileClass();
		if (ProjectileClassToUse)
		{
			UE_LOG(LogTemp, Log, TEXT("%s: 使用元素特定投掷物类 (元素: %d, 类: %s)"),
				*GetName(), (int32)CurrentElement, *ProjectileClassToUse->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("%s: 当前元素 %d 没有特定投掷物类"),
				*GetName(), (int32)CurrentElement);
		}
	}

	// 如果没有元素特定的投掷物，使用默认的AI投掷物类
	if (!ProjectileClassToUse)
	{
		ProjectileClassToUse = AIProjectileClass;
		if (ProjectileClassToUse)
		{
			UE_LOG(LogTemp, Log, TEXT("%s: 使用默认AI投掷物类 (%s)"),
				*GetName(), *ProjectileClassToUse->GetName());
		}
	}

	// 检查是否有可用的投掷物类
	if (!ProjectileClassToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: 没有可用的投射物类，无法发射投射物"), *GetName());
		return;
	}

	// 获取发射位置
	FVector SpawnLocation;
	FRotator SpawnRotation; // 占位，不会被使用
	GetProjectileLaunchParams(SpawnLocation, SpawnRotation);

	// AI始终向自己的前方发射
	FVector ForwardDirection = GetActorForwardVector();

	// 设置生成参数
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 暂时使用默认旋转生成（会被InitializeLaunchWithAngle覆盖）
	ACombatProjectile* Projectile = GetWorld()->SpawnActor<ACombatProjectile>(
		ProjectileClassToUse,
		SpawnLocation,
		FRotator::ZeroRotator, // 暂时的，会被InitializeLaunchWithAngle覆盖
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

		// 计算到玩家的水平距离
		float DistanceToPlayer = 800.0f; // 默认中距离
		float HeightDifference = 0.0f;

		if (UWorld* World = GetWorld())
		{
			if (ACharacter* PlayerCharacter = Cast<ACharacter>(World->GetFirstPlayerController()->GetPawn()))
			{
				FVector ToPlayer = PlayerCharacter->GetActorLocation() - SpawnLocation;
				// 只计算水平距离
				DistanceToPlayer = FVector(ToPlayer.X, ToPlayer.Y, 0.0f).Size();
				// 计算高度差
				HeightDifference = ToPlayer.Z;
			}
		}

		// 基于投掷物速度计算发射角度
		float LaunchAngle = Projectile->CalculateAngleForDistance(DistanceToPlayer, HeightDifference);

		// 初始化发射（会设置正确的速度和朝向）
		Projectile->InitializeLaunchWithAngle(ForwardDirection, LaunchAngle);

		UE_LOG(LogTemp, Log, TEXT("%s: AI发射投掷物，目标距离=%.1f，计算角度=%.1f度"),
			*GetName(), DistanceToPlayer, LaunchAngle);

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

	// 旋转不再计算，仅作占位
	OutRotation = FRotator::ZeroRotator;
}

void AElementalCombatEnemy::ApplyHealing(float Healing, AActor* Healer)
{
	if (Healing <= 0.0f)
	{
		return;
	}

	float OldHP = CurrentHP;

	// 增加生命值，但不超过最大值
	CurrentHP = FMath::Clamp(CurrentHP + Healing, 0.0f, MaxHP);

	float ActualHealing = CurrentHP - OldHP;

	if (ActualHealing > 0.0f)
	{
		// 更新生命条
		if (LifeBarWidget)
		{
			LifeBarWidget->SetLifePercentage(CurrentHP / MaxHP);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: 恢复生命值 %.1f (%.1f -> %.1f / %.1f), 治疗者: %s"),
			*GetName(), ActualHealing, OldHP, CurrentHP, MaxHP,
			Healer ? *Healer->GetName() : TEXT("Unknown"));
	}
}

void AElementalCombatEnemy::UpdateMaterialColors(FLinearColor Color)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: 无法获取骨骼网格组件"), *GetName());
		return;
	}

	// 获取所有材质实例
	TArray<UMaterialInterface*> Materials = MeshComp->GetMaterials();

	for (int32 i = 0; i < Materials.Num(); i++)
	{
		UMaterialInterface* Material = Materials[i];
		if (!Material)
		{
			continue;
		}

		// 尝试获取现有的动态材质实例
		UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Material);

		// 如果不是动态材质实例，创建一个
		if (!DynMaterial)
		{
			DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
			if (DynMaterial)
			{
				MeshComp->SetMaterial(i, DynMaterial);
			}
		}

		// 设置OverlayColor参数
		if (DynMaterial)
		{
			DynMaterial->SetVectorParameterValue(FName("OverlayColor"), Color);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s: 更新材质颜色为 (%.2f, %.2f, %.2f, %.2f)"),
		*GetName(), Color.R, Color.G, Color.B, Color.A);
}