// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WorldSaver.generated.h"

/**
 * Class that saves important information about the world
 */
UCLASS()
class CUBECRAFT_API UWorldSaver : public USaveGame
{
	GENERATED_BODY()
	
public:

	// WorldManager properties
	UPROPERTY()
	int centerX;
	UPROPERTY()
	int centerY;
	UPROPERTY()
	int32 seed;

	// TODO, save other important things like cubeSize, chunkSize, octaves ...

	// Player properties
	UPROPERTY()
		FVector playerLocation;
};
