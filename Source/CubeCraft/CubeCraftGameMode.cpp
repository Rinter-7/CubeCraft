// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CubeCraftGameMode.h"
#include "CubeCraftHUD.h"
#include "CubeCraftCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACubeCraftGameMode::ACubeCraftGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACubeCraftHUD::StaticClass();
}
