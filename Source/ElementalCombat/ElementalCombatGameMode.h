// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ElementalCombatGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AElementalCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AElementalCombatGameMode();
};



