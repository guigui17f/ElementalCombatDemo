// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;
class UNiagaraSystem;
class USoundBase;

/**
 * 投掷物配置结构
 */
USTRUCT(BlueprintType)
struct FProjectileConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float BaseDamage = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float InitialSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float MaxSpeed = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float GravityScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float LifeSpan = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	bool bShouldBounce = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	float BounceDamping = 0.5f;
};

/**
 * 战斗投掷物基类
 * 支持蓝图配置的投掷物系统
 */
UCLASS(Blueprintable)
class ELEMENTALCOMBAT_API ACombatProjectile : public AActor
{
	GENERATED_BODY()

public:
	ACombatProjectile();

protected:
	// 核心组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UParticleSystemComponent* ParticleComp;

	// 投掷物配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	FProjectileConfig ProjectileConfig;

	// 视觉效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	UNiagaraSystem* TrailEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	UNiagaraSystem* ImpactEffect;

	// 音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	USoundBase* LaunchSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ElementalCombat|Combat")
	USoundBase* ImpactSound;

	// 运行时属性
	UPROPERTY(BlueprintReadOnly, Category="ElementalCombat|Combat")
	float CurrentDamage;

	UPROPERTY(BlueprintReadOnly, Category="ElementalCombat|Combat")
	float SpeedMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category="ElementalCombat|Combat")
	float DamageMultiplier = 1.0f;

public:
	// 设置投掷物属性（根据蓄力时间调整）
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Combat")
	void SetProjectileProperties(float InSpeedMultiplier, float InDamageMultiplier);

	// 获取当前伤害值
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|Combat")
	float GetCurrentDamage() const { return CurrentDamage; }

	// 蓝图可调用的配置函数
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Combat")
	void SetProjectileConfig(const FProjectileConfig& NewConfig);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|Combat")
	FProjectileConfig GetProjectileConfig() const { return ProjectileConfig; }

	// 运行时修改投掷物速度
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Combat")
	void SetProjectileSpeed(float NewSpeed);

	// 运行时修改投掷物伤害
	UFUNCTION(BlueprintCallable, Category="ElementalCombat|Combat")
	void SetProjectileDamage(float NewDamage);

	// 获取当前速度
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ElementalCombat|Combat")
	float GetCurrentSpeed() const;

protected:
	// 碰撞处理
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, const FHitResult& Hit);

	// 应用伤害到目标
	void ApplyDamageToTarget(AActor* Target, const FHitResult& Hit);

	// 播放碰撞效果
	UFUNCTION(BlueprintImplementableEvent, Category="ElementalCombat|Combat")
	void PlayImpactEffects(const FVector& ImpactPoint, const FVector& ImpactNormal);

	// 初始化
	virtual void BeginPlay() override;
};