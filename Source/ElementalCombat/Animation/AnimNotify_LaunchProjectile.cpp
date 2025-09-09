// Copyright 2025 guigui17f. All Rights Reserved.

#include "AnimNotify_LaunchProjectile.h"
#include "Characters/AdvancedCombatCharacter.h"
#include "AI/ElementalCombatEnemy.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_LaunchProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_LaunchProjectile: No owner found"));
		return;
	}

	// Try casting to AdvancedCombatCharacter (player)
	if (AAdvancedCombatCharacter* PlayerCharacter = Cast<AAdvancedCombatCharacter>(Owner))
	{
		UE_LOG(LogTemp, Log, TEXT("AnimNotify_LaunchProjectile: Launching projectile for player %s"), *PlayerCharacter->GetName());
		PlayerCharacter->LaunchProjectile();
		return;
	}

	// Try casting to ElementalCombatEnemy (AI)
	if (AElementalCombatEnemy* EnemyCharacter = Cast<AElementalCombatEnemy>(Owner))
	{
		UE_LOG(LogTemp, Log, TEXT("AnimNotify_LaunchProjectile: Launching projectile for AI enemy %s"), *EnemyCharacter->GetName());
		EnemyCharacter->LaunchProjectile();
		return;
	}

	// If neither cast succeeded, log a warning
	UE_LOG(LogTemp, Warning, TEXT("AnimNotify_LaunchProjectile: Owner %s is not a supported character type"), *Owner->GetName());
}

FString UAnimNotify_LaunchProjectile::GetNotifyName_Implementation() const
{
	return FString("Launch Projectile");
}