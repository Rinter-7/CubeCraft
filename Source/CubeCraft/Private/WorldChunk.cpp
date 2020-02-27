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

void AWorldChunk::PrepareHISMs()
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
		PrepareHISMs();
		/*UCubeHISM* dirtInstances = *meshHISMs.Find("dirt");
		UCubeHISM* stoneInstances = *meshHISMs.Find("stone");
		UCubeHISM* diamondInstances = *meshHISMs.Find("diamond");
		UCubeHISM* iceInstances = *meshHISMs.Find("ice");

		for (int i = 0; i < chunkBuilder->transforms.Num(); ++i) {
			if (chunkBuilder->typeIndexes[i] == 0) {
				iceInstances->AddInstance(chunkBuilder->transforms[i]);
			} 
			else if (chunkBuilder->typeIndexes[i] == 1) {
				stoneInstances->AddInstance(chunkBuilder->transforms[i]);
			}
			else if (chunkBuilder->typeIndexes[i] == 2) {
				dirtInstances->AddInstance(chunkBuilder->transforms[i]);
			}
			else if (chunkBuilder->typeIndexes[i] == 3) {
				diamondInstances->AddInstance(chunkBuilder->transforms[i]);
			}
		}*/

		/*thread->Kill();
		thread = NULL;
		chunkBuilder.Release();*/
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

void AWorldChunk::SaveAndDestroy()
{
	saveAndDestroyTimerHandle.Invalidate();
	if (bIsActive)
		return;
	/*
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
				meshHISMs[i]->GetInstanceTransform(k,transform);
				saveChunk->locations.Push(transform.GetLocation());
				saveChunk->types.Push(i);
			}
		}

		FString slotname = FString::Printf(TEXT("ChunkX_%d_Y_%d"), x,y);

		// Start async save process.
		UGameplayStatics::AsyncSaveGameToSlot(saveChunk, slotname, 0, SavedDelegate);
	}*/

	//FBox2D(FVector2D(x - halfsize, y - halfsize), FVector2D(x + halfsize, y + halfsize));
	manager->quadTree->Remove(this, FBox2D(FVector2D(x - 0.1, y - 0.1), FVector2D(x + 0.1, y + 0.1)));
}

void AWorldChunk::SaveConfirm(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	if (!bSuccess) {
		UE_LOG(LogTemp, Warning, TEXT("Save failed"));
		SaveAndDestroy();
	}
	else {
		this->Destroy();
	}

}

void AWorldChunk::AddCube(FVector& position, FCubeType& type)
{
	// Find right hism component for this type

	// Transform that will be added to the hism
	FTransform transform;

	// Set scale of the cube
	transform.SetScale3D(FVector(manager->cubeSize / 99));
	
	// Relative position to actor
	position = position - GetActorLocation();

	// Set location in the tranform
	transform.SetLocation(position);

	// Finnaly add transform to the right HISM
	meshHISMs[0]->AddInstance(transform);
}


void AWorldChunk::BuildChunk(int x1, int y1, AWorldManager & worldManager)
{

	manager = &worldManager;

	SetActorLocation(FVector(x1 * worldManager.cubeSize * worldManager.chunkSize, y1 * worldManager.cubeSize * worldManager.chunkSize, 0));

	chunkBuilder = MakeUnique<FChunkBuilder>(x1, y1 ,worldManager,this);

	thread = FRunnableThread::Create(chunkBuilder.Get(),TEXT("ChunkBuilder"));

	if (thread == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("Chunkbuilder failed"));
	}
	
	GetWorldTimerManager().SetTimer(checkChunkBuilderTimer, this, &AWorldChunk::CheckChunkBuilder, 1, false, FMath::FRand());

	this->x = x1;
	this->y = y1;
}
