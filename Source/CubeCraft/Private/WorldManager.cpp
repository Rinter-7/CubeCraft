// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldManager.h"
#include "CubeCraft/Public/WorldChunk.h"
#include "Engine/World.h"
// Sets default values
AWorldManager::AWorldManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Init quad tree
	//quadTree = TQuadTree<AWorldChunk*> (FBox2D(0, FMath::Pow(2,20)), chunkSize*cubeSize);

}

// Called when the game starts or when spawned
void AWorldManager::BeginPlay()
{
	Super::BeginPlay();


	UWorld* world = GetWorld();
	for (int x = -nVisibleChunks + 1; x < nVisibleChunks; ++x) {
		for (int y = -nVisibleChunks + 1; y < nVisibleChunks; ++y) {
			AWorldChunk* chunk = world->SpawnActor<AWorldChunk>();
			//quadTree.Insert(chunk, FBox2D());
			chunk->octaves = octaves;
			chunk->persistance = persistance;
			chunk->heightAmplitude = heightAmplitude;
			chunk->BuildChunk(x, y, cubeSize, chunkSize);

		}
	}
	
}

// Called every frame
void AWorldManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

