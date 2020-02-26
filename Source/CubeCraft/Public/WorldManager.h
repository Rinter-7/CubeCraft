// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericQuadTree.h"
#include "WorldChunk.h"
#include "CubeType.h"
#include "WorldManager.generated.h"



UCLASS()
class CUBECRAFT_API AWorldManager : public AActor
{
	GENERATED_BODY()



	int centerX;
	int centerY;
	
	float time = 0;

	void AddChunk(int x, int y);

	void RemoveChunk(int x, int y);

	FBox2D BoxAroundPoint(int x, int y, float size);

	class APlayerController * playerController = nullptr;

	FTimerHandle checkPlayerPositionTimerHandle;

	// chunk length in ue units, used by CheckplayerPosition function, chunksize * cubesize
	float chunkLength;

public:	
	//Quad tree for efficient manipulation with world chunks
	TSharedPtr<TQuadTree<AWorldChunk*>> quadTree;

	// Sets default values for this actor's properties
	AWorldManager();

	//Updates the world around new point x,y 
	void RecenterWorld(int newCenterX, int newCenterY);

	//Check player position repeatedly and recenter world if needed
	void CheckPlayerPosition();

	// Adds a cube into the world, to the apropriete chunk
	void AddCube(FVector & position, FCubeType& type);

	UPROPERTY(EditAnywhere)
	TArray<FCubeType> types;

	// Size of one cube in the world
	UPROPERTY(EditAnywhere)
	float cubeSize = 80;

	// Size of one chunk in cubes
	UPROPERTY(EditAnywhere)
	int chunkSize = 32;

	// How many chunks are visible for player in one direction, the first chunk is the chunk player is standing on
	UPROPERTY(EditAnywhere)
	int nVisibleChunks = 5;

	// Seed for the perlin noise
	UPROPERTY(EditAnywhere)
		int32 seed;

	// Height of the floor
	UPROPERTY(EditAnywhere)
		float floorHeight = -1000;


	// Persistance of the perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin2D)
		float persistance = 2;

	// Height multiplier for perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin2D)
		float heightAmplitude = 800;

	// Number of octaves for perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin2D)
		int octaves = 2;

	// Persistance of the 3D perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin3D)
		float persistance3D = 2;

	// Number of octaves for 2D perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin3D)
		int octaves3D = 2;

	// value used to normilize height between -1 and 1
	UPROPERTY(EditAnywhere, Category = Perlin3D)
		int zDivisor = 800;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
