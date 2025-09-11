// Copyright 2025 guigui17f. All Rights Reserved.

#include "AdvancedCombatCharacter.h"
#include "Combat/Projectiles/CombatProjectile.h"
#include "Combat/Elemental/ElementalComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"
#include "EnhancedInputComponent.h"
#include "Variant_Combat/UI/CombatLifeBar.h"

AAdvancedCombatCharacter::AAdvancedCombatCharacter()
{
	// 默认配置
	ProjectileSocketName = TEXT("hand_r");

	// 创建元素组件
	ElementalComponent = CreateDefaultSubobject<UElementalComponent>(TEXT("ElementalComponent"));
}

void AAdvancedCombatCharacter::DoChargedAttackStart()
{
	// 调用父类实现（播放动画等）
	Super::DoChargedAttackStart();

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

	// 计算蓄力时间
	float ChargeTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	
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
		ElementProjectileClass, 
		SpawnLocation, 
		SpawnRotation, 
		SpawnParams
	);

	if (Projectile)
	{
		// 根据蓄力时间调整投掷物属性
		float SpeedMultiplier = 1.0f;
		float DamageMultiplier = 1.0f;

		// 使用曲线计算倍率
		if (ChargeSpeedCurve)
		{
			SpeedMultiplier = ChargeSpeedCurve->GetFloatValue(ChargeTime);
		}
		else
		{
			// 默认的蓄力倍率计算
			SpeedMultiplier = GetChargeMultiplier(ChargeTime);
		}

		if (ChargeDamageCurve)
		{
			DamageMultiplier = ChargeDamageCurve->GetFloatValue(ChargeTime);
		}
		else
		{
			// 默认的蓄力倍率计算
			DamageMultiplier = GetChargeMultiplier(ChargeTime);
		}

		// 获取当前元素的效果数据并应用到伤害倍率
		const FElementalEffectData* ElementData = ElementalComponent->GetElementEffectDataPtr(CurrentElement);
		if (ElementData)
		{
			// 应用元素的伤害倍率（金元素效果）
			DamageMultiplier *= ElementData->DamageMultiplier;
		}

		// 应用倍率到投掷物
		Projectile->SetProjectileProperties(SpeedMultiplier, DamageMultiplier);

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
		
		UE_LOG(LogTemp, Log, TEXT("%s 发射了 %s 元素投掷物，伤害倍率: %.2f"), 
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

	// 获取发射方向（使用控制器旋转）
	if (GetController())
	{
		OutRotation = GetControlRotation();
	}
	else
	{
		// 如果没有控制器，使用角色朝向
		OutRotation = GetActorRotation();
	}
}