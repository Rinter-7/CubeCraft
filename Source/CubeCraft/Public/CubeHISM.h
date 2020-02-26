// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "CubeHISM.generated.h"

/**
 * 
 */
UCLASS()
class CUBECRAFT_API UCubeHISM : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

	TMap<int32, float> damagedCubes;

	void RemoveCube(int32 item);

public:
	float cubeMaxHealth = 5;

	void DamageCube(float damage, int32 item);

	void HealCube(int32 item);
};
