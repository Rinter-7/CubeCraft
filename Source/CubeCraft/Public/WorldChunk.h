// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericQuadTree.h"
#include "CubeType.h"
#include "ChunkBuilder.h"
#include "WorldChunk.generated.h"

/**
* The world is divided into chunks to be more easily managable, each chunk takes care of its own piece of land
*/
UCLASS()
class CUBECRAFT_API AWorldChunk : public AActor
{
	GENERATED_BODY()

	// This map lets us use hism for every new static mesh we add to the chunk
	// Hisms are usefull for rendering many instances of the same static mesh
	TMap<FString,class UCubeHISM* > meshHISMs;

	void PrepareHISMs();

	void PrepareHISM(class UCubeHISM* hism);

	bool bIsActive;

	float destroyTime = 10;

	FTimerHandle saveAndDestroyTimerHandle;

	TUniquePtr<FChunkBuilder> chunkBuilder = NULL;

	FRunnableThread* thread = NULL;

	FTimerHandle checkChunkBuilderTimer;

	void CheckChunkBuilder();


public:	

	class USceneComponent* root;

	// Sets default values for this actor's properties
	AWorldChunk();

	// Loads chunk from memory if any, returns false if it doesnt exist in memory
	// Avoid calling this function multiple times before unload is called, it would only load duplicates of this chunk
	bool LoadChunk(int x, int y);

	// Saves this chunk into memory
	void SaveChunk();

	// Destroys this chunk
	void DisableChunk();

	// Activates chunk
	void ActivateChunk();

	// Saves this chunk into memory and destroys it
	void SaveAndDestroy();

	// Position of the chunk, in custom coordinate system.
	int x;
	int y;

	// Manager of this chunk
	class AWorldManager * manager;

	// Function that adds new mesh instance to the chunk, this function doesnt check if the mesh is withing chunk bounderies, 
	// check bounderies beforehand. 
	void AddMeshInstanceToChunk(UStaticMesh & staticMesh, FTransform & transform);

	void AddCube(FVector& position, FCubeType & type);

	// Builds chunk dependent on given coordinates and cubeSize
	void BuildChunk(int x, int y, class  AWorldManager & worldManager);

	void CubeRemovedAt(FTransform & trans);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
