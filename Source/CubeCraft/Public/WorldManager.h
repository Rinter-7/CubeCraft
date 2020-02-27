// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericQuadTree.h"
#include "WorldChunk.h"
#include "CubeType.h"
#include "WorldManager.generated.h"


/**
* World Managers job is to build the world around player using world chunks, it can also save the world to disk
*/
UCLASS()
class CUBECRAFT_API AWorldManager : public AActor
{
	GENERATED_BODY()

	// Center of the world (Player should always be in the center)
	int centerX = 0;
	int centerY = 0;
	
	// Correct player position when loading
	FVector playerPosition;
	void ResetPlayerPosition();

	// Coordinates of removed chunks, used for cleanup
	TArray<FIntPoint> removedChunks;

	// Add chunk at specified coordinates
	void AddChunk(int x, int y);

	// Removes chunk at specified coordinates
	void RemoveChunk(int x, int y);

	// Creates small box aroun given point
	FBox2D BoxAroundPoint(int x, int y, float size);

	// Player controller, used by check player position
	class APlayerController * playerController = nullptr;

	// Timer handle for check player position function
	FTimerHandle checkPlayerPositionTimerHandle;

	// chunk length in ue units, used by CheckplayerPosition function, chunksize * cubesize
	float chunkLength;

	// If there isnt performed a safe the saved chunks should be destroyd
	bool ShouldDestroy = true;

public:	
	//Quad tree for efficient manipulation with world chunks
	TSharedPtr<TQuadTree<AWorldChunk*>> quadTree;

	void SaveWorld();

	void LoadWorld();

	// Sets default values for this actor's properties
	AWorldManager();

	//Updates the world around new point x,y 
	void RecenterWorld(int newCenterX, int newCenterY);

	//Check player position repeatedly and recenter world if needed
	void CheckPlayerPosition();

	// Adds a cube into the world, to the apropriete chunk
	void AddCube(FVector & position, int typeIndex);

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

	// No cubes will be generated under this height
	UPROPERTY(EditAnywhere)
		float floorHeight = -1000;


	// Persistance of the perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin2D)
		float persistance = 2;

	// Height multiplier for perlin noise, perlin noise is between 1 and -1
	UPROPERTY(EditAnywhere, Category = Perlin2D)
		float heightAmplitude = 800;

	// Number of octaves for perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin2D)
		int octaves = 2;

	// Persistance of the 3D perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin3D)
		float persistance3D = 2;

	// Number of octaves for 3D perlin noise
	UPROPERTY(EditAnywhere, Category = Perlin3D)
		int octaves3D = 2;

	// value used to normilize height between -1 and 1
	UPROPERTY(EditAnywhere, Category = Perlin3D)
		int zDivisor = 800;

	// The world name used for saving and and loading,
	// if there is saved world with this name it will be loaded
	// After you leave world you need to press P for saving if you want that world to be loaded
	UPROPERTY(EditAnywhere, Category = SaveAndLoad)
		FString worldName = "Default";

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called before this object ceases to exit
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
