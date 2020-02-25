// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "CubeHISM.generated.h"

/**
 * 
 */
UCLASS()
class CUBECRAFT_API UCubeHISM : public UHierarchicalInstancedStaticMeshComponent
{
	GENERATED_BODY()

		
	TMap<int32, float> damagedCubes;

public:
	float cubeMaxHealth = 5;

	UCubeHISM();

	void DestroyCube(float damage, int32 item);

	void HealCube(int32 item);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	
};
