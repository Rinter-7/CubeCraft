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

	class AWorldChunk * owner;

	void RemoveCube(int32 item);

public:
	float cubeMaxHealth = 5;

	UCubeHISM();

	void DamageCube(float damage, int32 item);

	void HealCube(int32 item);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
