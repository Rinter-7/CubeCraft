// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ChunkSaver.generated.h"

/**
 * Class that saves chunk information
 */
UCLASS()
class CUBECRAFT_API UChunkSaver : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FVector> locations;

	UPROPERTY()
	TArray<int> types;

	UPROPERTY()
		float cubeSize;
	
};
