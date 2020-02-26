// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericQuadTree.h"
#include "CubeType.h"
#include "ChunkBuilder.h"
#include "ColumnTransform.h"
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

	TSharedPtr<TArray<FColumnTransform>> columnMatrix = NULL;

	// Prepare default hisms
	void PrepareHISMs();

	// If true the object is active and can be seen in the world, if false the object is hidden
	bool bIsActive;

	// Maximum time this object can be inactive, if it is inactive for 10 seconds it destroys itself
	float destroyTime = 10;

	// Timer handle for destroy function
	FTimerHandle saveAndDestroyTimerHandle;

	// Pointer to chunkbuilder, this class will be operating on separete thread and prepares transforms for this chunk
	TUniquePtr<FChunkBuilder> chunkBuilder = NULL;

	// Actual thread the chunkbuilder will run on
	FRunnableThread* thread = NULL;

	// Timer handle for checkChunkBuilder function
	FTimerHandle checkChunkBuilderTimer;

	// This function checks chunkbuilder and when the chunk builder is finished it copies transforms from it
	void CheckChunkBuilder();


public:	

	// Root of this actor
	class USceneComponent* root;

	// Sets default values for this actor's properties
	AWorldChunk();

	// Loads chunk from memory if any, returns false if it doesnt exist in memory
	// Avoid calling this function multiple times before unload is called, it would only load duplicates of this chunk
	bool LoadChunk(int x, int y);

	// Saves this chunk into memory
	void SaveChunk();

	// Disables this chunk
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

	void AddCube(FVector& position, FCubeType & type);

	void AddCubePrecisePosition(FVector const & position, FCubeType& type);

	// Builds chunk dependent on given coordinates and cubeSize
	void BuildChunk(int x, int y, class  AWorldManager & worldManager);

	void CubeRemovedAt(FTransform & trans);

	void AddCubeFromColumn(FVector const &location, int xCol, int yCol);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
