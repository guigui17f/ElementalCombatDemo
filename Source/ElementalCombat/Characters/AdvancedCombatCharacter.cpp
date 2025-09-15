// Copyright 2025 guigui17f. All Rights Reserved.

#include "AdvancedCombatCharacter.h"
#include "Combat/Projectiles/CombatProjectile.h"
#include "Combat/Elemental/ElementalComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"
#include "EnhancedInputComponent.h"
#include "Variant_Combat/UI/CombatLifeBar.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

AAdvancedCombatCharacter::AAdvancedCombatCharacter()
{
	// 默认配置
	ProjectileSocketName = TEXT("hand_r");

	// 创建元素组件
	ElementalComponent = CreateDefaultSubobject<UElementalComponent>(TEXT("ElementalComponent"));
}

void AAdvancedCombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 设置初始元素为土元素
	if (ElementalComponent)
	{
		ElementalComponent->SwitchElement(EElementalType::Earth);

		// 应用初始元素颜色到材质
		const FElementalEffectData* ElementData = ElementalComponent->GetElementEffectDataPtr(EElementalType::Earth);
		if (ElementData)
		{
			UpdateMaterialColors(ElementData->ElementColor);
		}

		UE_LOG(LogTemp, Log, TEXT("%s: 初始化为土元素"), *GetName());
	}
}

void AAdvancedCombatCharacter::DoChargedAttackStart()
{
	// 设置蓄力攻击状态
	bIsChargingAttack = true;

	// 如果正在攻击中，缓存输入时间
	if (bIsAttacking)
	{
		CachedAttackInputTime = GetWorld()->GetTimeSeconds();
		return;
	}

	// 执行远程攻击而不是蓄力攻击
	DoRangedAttack();

	// 记录蓄力开始时间
	ChargeStartTime = GetWorld()->GetTimeSeconds();
}

void AAdvancedCombatCharacter::DoChargedAttackEnd()
{
	// 不调用父类的近战攻击实现
	// 改为发射投掷物
	
	// 结束蓄力状态
	bIsChargingAttack = false;
}

void AAdvancedCombatCharacter::LaunchProjectile()
{
	if (!ElementalComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementalComponent not found on %s"), *GetName());
		return;
	}

	// 获取当前元素
	EElementalType CurrentElement = ElementalComponent->GetCurrentElement();
	
	// 尝试获取元素特定的投掷物类
	UClass* ElementProjectileClass = ElementalComponent->GetCurrentProjectileClass();
	if (!ElementProjectileClass)
	{
		// 如果没有元素特定的投掷物，使用默认的
		ElementProjectileClass = ProjectileClass;
	}

	// 检查是否有投掷物类
	if (!ElementProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("No ProjectileClass available for %s"), *GetName());
		return;
	}

	// 获取发射位置
	FVector SpawnLocation;
	FRotator SpawnRotation; // 占位，不会被使用
	GetProjectileLaunchParams(SpawnLocation, SpawnRotation);

	// 获取前向方向（用于计算初始朝向）
	FVector ForwardDirection;
	if (GetController())
	{
		// 使用控制器的yaw方向，忽略pitch
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.0f;
		ForwardDirection = ControlRot.Vector();
	}
	else
	{
		ForwardDirection = GetActorForwardVector();
	}

	// 计算15度仰角的初始朝向
	FVector LaunchDirection = ForwardDirection;
	LaunchDirection.Z = 0.0f;
	LaunchDirection.Normalize();
	float AngleRad = FMath::DegreesToRadians(15.0f);
	LaunchDirection = LaunchDirection * FMath::Cos(AngleRad) + FVector::UpVector * FMath::Sin(AngleRad);
	FRotator InitialRotation = LaunchDirection.Rotation();

	// 设置生成参数
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 生成投掷物（使用计算的初始朝向）
	ACombatProjectile* Projectile = GetWorld()->SpawnActor<ACombatProjectile>(
		ElementProjectileClass,
		SpawnLocation,
		InitialRotation, // 朝向飞行方向
		SpawnParams
	);

	if (Projectile)
	{
		// 设置投掷物属性（不受蓄力时间影响）
		float SpeedMultiplier = 1.0f;  // 固定速度倍率
		float DamageMultiplier = 1.0f; // 基础伤害倍率

		// 获取当前元素的效果数据并应用到伤害倍率
		const FElementalEffectData* ElementData = ElementalComponent->GetElementEffectDataPtr(CurrentElement);
		if (ElementData)
		{
			// 应用元素的伤害倍率（金元素效果）
			DamageMultiplier = ElementData->DamageMultiplier;
		}

		// 应用倍率到投掷物
		Projectile->SetProjectileProperties(SpeedMultiplier, DamageMultiplier);

		// 初始化发射（会设置速度并确保朝向正确）
		Projectile->InitializeLaunchWithAngle(ForwardDirection, 15.0f);

		// 触发蓝图事件
		OnProjectileLaunched(Projectile);

		// 输出调试信息
		FString ElementName;
		switch (CurrentElement)
		{
		case EElementalType::Metal: ElementName = TEXT("金"); break;
		case EElementalType::Wood: ElementName = TEXT("木"); break;
		case EElementalType::Water: ElementName = TEXT("水"); break;
		case EElementalType::Fire: ElementName = TEXT("火"); break;
		case EElementalType::Earth: ElementName = TEXT("土"); break;
		default: ElementName = TEXT("无"); break;
		}
		
		UE_LOG(LogTemp, Log, TEXT("%s 发射了 %s 元素投掷物，元素伤害倍率: %.2f"),
			*GetName(), *ElementName, DamageMultiplier);
	}

	// 重置蓄力时间
	ChargeStartTime = 0.0f;
}

void AAdvancedCombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 调用父类实现
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 绑定输入
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 跳跃输入绑定
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// 元素切换 - 1-5键
		if (SwitchToMetalAction)
		{
			EnhancedInputComponent->BindAction(SwitchToMetalAction, ETriggerEvent::Started, this, &AAdvancedCombatCharacter::SwitchToMetal);
		}
		if (SwitchToWoodAction)
		{
			EnhancedInputComponent->BindAction(SwitchToWoodAction, ETriggerEvent::Started, this, &AAdvancedCombatCharacter::SwitchToWood);
		}
		if (SwitchToWaterAction)
		{
			EnhancedInputComponent->BindAction(SwitchToWaterAction, ETriggerEvent::Started, this, &AAdvancedCombatCharacter::SwitchToWater);
		}
		if (SwitchToFireAction)
		{
			EnhancedInputComponent->BindAction(SwitchToFireAction, ETriggerEvent::Started, this, &AAdvancedCombatCharacter::SwitchToFire);
		}
		if (SwitchToEarthAction)
		{
			EnhancedInputComponent->BindAction(SwitchToEarthAction, ETriggerEvent::Started, this, &AAdvancedCombatCharacter::SwitchToEarth);
		}
	}
}

void AAdvancedCombatCharacter::SwitchToElement(EElementalType NewElement)
{
	if (!ElementalComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementalComponent not found on %s"), *GetName());
		return;
	}

	// 切换元素
	ElementalComponent->SwitchElement(NewElement);

	// 应用元素颜色到材质
	const FElementalEffectData* ElementData = ElementalComponent->GetElementEffectDataPtr(NewElement);
	if (ElementData)
	{
		UpdateMaterialColors(ElementData->ElementColor);
	}

	// 输出调试信息
	FString ElementName;
	switch (NewElement)
	{
	case EElementalType::Metal:
		ElementName = TEXT("金");
		break;
	case EElementalType::Wood:
		ElementName = TEXT("木");
		break;
	case EElementalType::Water:
		ElementName = TEXT("水");
		break;
	case EElementalType::Fire:
		ElementName = TEXT("火");
		break;
	case EElementalType::Earth:
		ElementName = TEXT("土");
		break;
	default:
		ElementName = TEXT("无");
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("%s 切换到 %s 元素"), *GetName(), *ElementName);
}

float AAdvancedCombatCharacter::GetChargeMultiplier(float ChargeTime) const
{
	// 默认的蓄力倍率计算
	if (ChargeTime < 0.5f)
	{
		return 1.0f;
	}
	else if (ChargeTime < 1.0f)
	{
		return 1.5f;
	}
	else
	{
		return 2.0f;
	}
}

float AAdvancedCombatCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, 
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
		// 震屏效果应该在蓝图的ReceivedDamage事件中处理
	}

	return Damage;
}

void AAdvancedCombatCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	// 不需要重置ragdoll，因为我们没有启用它
}

void AAdvancedCombatCharacter::GetProjectileLaunchParams(FVector& OutLocation, FRotator& OutRotation) const
{
	// 获取发射位置（从手部Socket）
	if (GetMesh() && GetMesh()->DoesSocketExist(ProjectileSocketName))
	{
		OutLocation = GetMesh()->GetSocketLocation(ProjectileSocketName);
	}
	else
	{
		// 如果没有Socket，使用角色位置加偏移
		OutLocation = GetActorLocation() + FVector(0, 0, 100.0f);
	}

	// 旋转不再计算，仅作占位
	OutRotation = FRotator::ZeroRotator;
}

void AAdvancedCombatCharacter::ApplyHealing(float Healing, AActor* Healer)
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

void AAdvancedCombatCharacter::UpdateMaterialColors(FLinearColor Color)
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

void AAdvancedCombatCharacter::DoRangedAttack()
{
	// 设置攻击状态
	bIsAttacking = true;

	// 播放远程攻击动画蒙太奇
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (RangedAttackMontage)
		{
			const float MontageLength = AnimInstance->Montage_Play(RangedAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

			// 设置动画结束代理
			if (MontageLength > 0.0f)
			{
				AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, RangedAttackMontage);
			}

			UE_LOG(LogTemp, Log, TEXT("%s: 播放远程攻击动画，长度: %.2f秒"), *GetName(), MontageLength);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: 远程攻击动画蒙太奇未设置"), *GetName());
			// 如果没有动画，直接结束攻击状态
			bIsAttacking = false;
		}
	}
}