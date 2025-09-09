// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_LaunchProjectile.generated.h"

/**
 * AnimNotify to tell combat characters to launch a projectile.
 * Supports both AdvancedCombatCharacter (player) and ElementalCombatEnemy (AI).
 */
UCLASS()
class ELEMENTALCOMBAT_API UAnimNotify_LaunchProjectile : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	/** Perform the Anim Notify */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** Get the notify name */
	virtual FString GetNotifyName_Implementation() const override;
};