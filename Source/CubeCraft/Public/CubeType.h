// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CubeType.generated.h"

/**
 * Structure that helps to keep information about one cube type in one place
 */
USTRUCT(BlueprintType)
struct CUBECRAFT_API FCubeType 
{

	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere)
		FString name;

		UPROPERTY(EditAnywhere)
		class UStaticMesh* cubeMesh;

		UPROPERTY(EditAnywhere)
		float maxHealth;
};
