// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CubeType.generated.h"

/**
 * 
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
		float breakTime;
};
