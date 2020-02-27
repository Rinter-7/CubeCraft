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

	// This array lets us use hism for every new cube type we add to the chunk
	// Hisms are usefull for rendering many instances of the same static mesh
		UPROPERTY()
	TArray<class UCubeHISM*> meshHISMs;

	// Copies hism componenets from the chunkbuilder
	void CopyAndBindHISMs();

	// If true the object is active and can be seen in the world, if false the object is hidden
	bool bIsActive = true;

	// Maximum time this object can be inactive, if it is inactive for 10 seconds it destroys itself, and saves itself on disk
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

	// Manager of this chunk and this world
	class AWorldManager* manager;

public:	

	// Root of this actor
	class USceneComponent* root;

	// Sets default values for this actor's properties
	AWorldChunk();

	// Disables this chunk, chunk will kill itself 10 sec after disabling if it isnt reactivated again in the meantime
	void DisableChunk();

	// Activates chunk
	void ActivateChunk();

	// Saves this chunk to memory asyncly
	bool SaveChunk();

	// Saves this chunk into memory and destroys it
	void SaveAndDestroy();

	// Function called after saving is completed
	void SaveConfirm(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	// Position of the chunk, in custom coordinate system.
	int x;
	int y;

	// Adds cube of type to the chunk
	void AddCube(FVector& position, int typeIndex);

	// Builds chunk dependent on given coordinates and cubeSize
	void BuildChunk(int x, int y, class  AWorldManager & worldManager);
};
