// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ColumnTransform.generated.h"

/**
 * 
 */
USTRUCT()
struct CUBECRAFT_API FColumnTransform
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		FVector topLocation;
	UPROPERTY()
		TArray<int> types;
};