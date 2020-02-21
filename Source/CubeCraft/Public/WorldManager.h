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



	int centerX;
	int centerY;
	
	float time = 0;

	void AddChunk(int x, int y);

	void RemoveChunk(int x, int y);

	FBox2D BoxAroundPoint(int x, int y, float size);

	class APlayerController * playerController = nullptr;

	FTimerHandle checkPlayerPositionTimerHandle;

	// chunk length in ue units, used by CheckplayerPosition function, chunksize * cubesize/2
	float chunkLength;

public:	
	//Quad tree for efficient manipulation with world chunks
	TSharedPtr<TQuadTree<AWorldChunk*>> quadTree;

	float ModifiedPerlin(float x, float y) const;

	// Sets default values for this actor's properties
	AWorldManager();

	//Updates the world around new point x,y 
	void RecenterWorld(int newCenterX, int newCenterY);

	//Check player position repeatedly and recenter world if needed
	void CheckPlayerPosition();

	// Adds a cube into the world, to the apropriete chunk
	void AddCube(FVector & position, const FString & type = "Basic" );

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
