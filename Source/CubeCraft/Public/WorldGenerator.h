// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGenerator.generated.h"


/**
* This class generates the world map
*/
UCLASS()
class CUBECRAFT_API AWorldGenerator : public AActor
{
	GENERATED_BODY()

		TArray<AActor*> cubes;

	//Variable that holds mesh for the world piece
	class UStaticMesh * cubeMesh;

	// Function that generates 2D perlin noise from 2D seedArr and stores it in outputArr
	void PerlinNoise2D(float * seedArr, float * outputArr);

	void InitializeSeedArray(int size, float * seedArr);
public:	
	// Sets default values for this actor's properties
	AWorldGenerator();

	// Generates the world
	void GenerateWorld();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	

protected:
	// Called when actor is placed in the scene
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere)
	bool isGenerated = false;
	// Properties that define starting size of the world map
	UPROPERTY(EditAnywhere)
		int32 worldWidth = 32;
		
	// Properties that define starting size of the world map
	UPROPERTY(EditAnywhere)
		int32 worldLength = 32;

	// bias for perlin noise
	UPROPERTY(EditAnywhere)
		float bias = 2;

	// Number of octaves for perlin noise
	UPROPERTY(EditAnywhere)
		int octaves = 2;

	// Use if you want to replicate one particular world
	UPROPERTY(EditAnywhere)
		bool bUseCustomSeed = false;

	// Seed used by random function used in world generation
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bUseCustomSeed"))
		uint32 seed = 0;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
