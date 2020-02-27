// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "CubeHISM.generated.h"

/**
 * Wrap around ISM or HISM, Performence tests should be done to determine which is better
 */
UCLASS()
class CUBECRAFT_API UCubeHISM : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

	// Keep track of cubes being currently damaged
	TMap<int32, float> damagedCubes;

	// Remove cube from the ISM/HISM
	void RemoveCube(int32 item);

public:

	// Health of cubes of this type
	float cubeMaxHealth = 5;

	// Apply damage to specific cube
	void DamageCube(float damage, int32 item);

	// Heal specific cube, we want to heal cubes that stopped taking damage
	void HealCube(int32 item);
};
