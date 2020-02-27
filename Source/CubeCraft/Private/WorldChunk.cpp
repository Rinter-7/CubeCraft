// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldChunk.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "CubeCraft/Public/WorldManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "CubeHISM.h"
#include "CubeCraft/Public/MyPerlin.h"
#include "HAL/RunnableThread.h"
#include "ChunkSaver.h"
#include "Kismet/GameplayStatics.h"

void AWorldChunk::CopyAndBindHISMs()
{
	for (int k = 0; k < chunkBuilder->cubeComps.Num(); k++) {
		meshHISMs.Push(chunkBuilder->cubeComps[k]);
		chunkBuilder->cubeComps[k] = nullptr;
	}

	int i = 0;
	for (auto && it : meshHISMs) {
		it->RegisterComponentWithWorld(GetWorld());
		it->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		it->SetStaticMesh(manager->types[i].cubeMesh);
		it->cubeMaxHealth = manager->types[i++].maxHealth;

	}
}


void AWorldChunk::CheckChunkBuilder()
{
	checkChunkBuilderTimer.Invalidate();
	if (chunkBuilder->IsFinished()) {
		CopyAndBindHISMs();
		thread->Kill();
		thread = NULL;
		chunkBuilder.Release();
	}
	else {
		GetWorldTimerManager().SetTimer(checkChunkBuilderTimer, this, &AWorldChunk::CheckChunkBuilder, 1, false, FMath::FRand());
	}
}

AWorldChunk::AWorldChunk()
{
	root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	RootComponent = root;
	//root->Mobility = EComponentMobility::Static;

	PrimaryActorTick.bCanEverTick = false;
}



void AWorldChunk::DisableChunk()
{
	SetActorHiddenInGame(true);
	bIsActive = false;
	GetWorld()->GetTimerManager().SetTimer(saveAndDestroyTimerHandle, this, &AWorldChunk::SaveAndDestroy, destroyTime, false);
}

void AWorldChunk::ActivateChunk()
{
	bIsActive = true;

	if (IsHidden()) {
		SetActorHiddenInGame(false);
		return;
	}
}

bool AWorldChunk::SaveChunk()
{
	// Try to save this chunk
	if (UChunkSaver* saveChunk = Cast<UChunkSaver>(UGameplayStatics::CreateSaveGameObject(UChunkSaver::StaticClass())))
	{

		// Set up the (optional) delegate.
		FAsyncSaveGameToSlotDelegate SavedDelegate;

		// USomeUObjectClass::SaveGameDelegateFunction is a void function that takes the following parameters: const FString& SlotName, const int32 UserIndex, bool bSuccess
		SavedDelegate.BindUObject(this, &AWorldChunk::SaveConfirm);

		// Set data on the savegame object.

		for (int i = 0; i < meshHISMs.Num(); ++i) {
			int count = meshHISMs[i]->GetInstanceCount();
			FTransform transform;
			for (int k = 0; k < count; ++k) {
				meshHISMs[i]->GetInstanceTransform(k, transform);
				saveChunk->locations.Push(transform.GetLocation());
				saveChunk->types.Push(i);
			}
		}

		FString slotname = manager->worldName + FString::Printf(TEXT("\\ChunkX_%d_Y_%d"), x, y);

		// Start async save process.
		UGameplayStatics::AsyncSaveGameToSlot(saveChunk, slotname, 0, SavedDelegate);
		return true;
	}
	return false;
}

void AWorldChunk::SaveAndDestroy()
{
	saveAndDestroyTimerHandle.Invalidate();
	if (bIsActive)
		return;
	
	if(!SaveChunk()){
		this->Destroy(); // Destroy it if save failed
	}

	manager->quadTree->Remove(this, FBox2D(FVector2D(x - 0.1, y - 0.1), FVector2D(x + 0.1, y + 0.1)));
}

void AWorldChunk::SaveConfirm(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	if (!bSuccess) { // Save failed try again
		UE_LOG(LogTemp, Warning, TEXT("Save failed"));
		SaveChunk();
	}
	else if(!bIsActive){ // Save was succesfull and we are inactive, destroy this
		this->Destroy();
	}
}

void AWorldChunk::AddCube(FVector& position, int typeIndex)
{
	// Find right hism component for this type

	// Transform that will be added to the hism
	FTransform transform;

	// Set scale of the cube
	transform.SetScale3D(FVector(manager->cubeSize / 99.9));
	
	// Relative position to actor
	position = position - GetActorLocation();

	// Set location in the tranform
	transform.SetLocation(position);

	// Finnaly add transform to the right HISM
	meshHISMs[typeIndex]->AddInstance(transform);
}


void AWorldChunk::BuildChunk(int x1, int y1, AWorldManager & worldManager)
{
	manager = &worldManager;

	SetActorLocation(FVector(x1 * worldManager.cubeSize * worldManager.chunkSize, y1 * worldManager.cubeSize * worldManager.chunkSize, 0));

	chunkBuilder = MakeUnique<FChunkBuilder>(x1, y1 ,worldManager,this);

	// Start thread that builds this chunk
	thread = FRunnableThread::Create(chunkBuilder.Get(),TEXT("ChunkBuilder"));

	if (thread == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("Chunkbuilder failed"));
	}
	
	// Check if thread that is building this object has finished, randomize so all objects wont finish at once
	GetWorldTimerManager().SetTimer(checkChunkBuilderTimer, this, &AWorldChunk::CheckChunkBuilder, 1, false, FMath::FRand());

	this->x = x1;
	this->y = y1;
}
