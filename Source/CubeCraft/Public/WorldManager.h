// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericQuadTree.h"
#include "WorldChunk.h"
#include "WorldManager.generated.h"



UCLASS()
class CUBECRAFT_API AWorldManager : public AActor
{
	GENERATED_BODY()

		//Quad tree for efficient manipulation with world chunks
		//TQuadTree<AWorldChunk*> quadTree;
	
public:	
	// Sets default values for this actor's properties
	AWorldManager();

	// Size of one cube in the world
	UPROPERTY(EditAnywhere)
	float cubeSize = 80;

	// Size of one chunk in cubes
	UPROPERTY(EditAnywhere)
	int chunkSize = 32;

	// How many chunks are visible for player in one direction, the first chunk is the chunk player is standing on
	UPROPERTY(EditAnywhere)
	int nVisibleChunks = 5;


	// bias for perlin noise
	UPROPERTY(EditAnywhere)
		float persistance = 2;

	// Height multiplier for perlin noise
	UPROPERTY(EditAnywhere)
		float heightAmplitude = 800;

	// Number of octaves for perlin noise
	UPROPERTY(EditAnywhere)
		int octaves = 2;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
