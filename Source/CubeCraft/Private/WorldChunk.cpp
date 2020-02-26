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





void AWorldChunk::PrepareHISMs()
{
	for (auto&& it : manager->types) {
		UCubeHISM* meshInstances = NewObject< UCubeHISM>(this);
		PrepareHISM(meshInstances);
		meshHISMs.Add(it.name, meshInstances);
		meshInstances->SetStaticMesh(it.cubeMesh);
		meshInstances->cubeMaxHealth = it.maxHealth;
	}
}

void AWorldChunk::PrepareHISM(UCubeHISM* hism)
{
	hism->RegisterComponent();
	hism->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	hism->SetCollisionProfileName(TEXT("Pawn"));
	hism->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
}

void AWorldChunk::CheckChunkBuilder()
{
	checkChunkBuilderTimer.Invalidate();
	if (chunkBuilder->IsFinished()) {
		UCubeHISM* dirtInstances = *meshHISMs.Find("dirt");
		UCubeHISM* stoneInstances = *meshHISMs.Find("stone");
		UCubeHISM* diamondInstances = *meshHISMs.Find("diamond");
		UCubeHISM* iceInstances = *meshHISMs.Find("ice");

		for (int i = 0; i < chunkBuilder->transforms.Num(); ++i) {
			if (chunkBuilder->types[i] == 0) {
				iceInstances->AddInstance(chunkBuilder->transforms[i]);
			} 
			else if (chunkBuilder->types[i] == 1) {
				stoneInstances->AddInstance(chunkBuilder->transforms[i]);
			}
			else if (chunkBuilder->types[i] == 2) {
				dirtInstances->AddInstance(chunkBuilder->transforms[i]);
			}
			else if (chunkBuilder->types[i] == 3) {
				diamondInstances->AddInstance(chunkBuilder->transforms[i]);
			}
		}
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

	PrimaryActorTick.bCanEverTick = false;
}


bool AWorldChunk::LoadChunk(int x1, int y1)
{
	return false;
}

void AWorldChunk::SaveChunk()
{
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

	//FBox2D(FVector2D(x - halfsize, y - halfsize), FVector2D(x + halfsize, y + halfsize));
	manager->quadTree->Remove(this, FBox2D(FVector2D(x - 0.1, y - 0.1), FVector2D(x + 0.1, y + 0.1)));
	this->Destroy();
}

void AWorldChunk::AddMeshInstanceToChunk(UStaticMesh& staticMesh, FTransform& transform)
{
}

void AWorldChunk::AddCube(FVector& position, FCubeType& type)
{
	// Find right hism component for this type
	UCubeHISM* meshInstances = *meshHISMs.Find(type.name);

	// Transform that will be added to the hism
	FTransform transform;

	// Cache cubesize for further use
	float cubeSize = manager->cubeSize;

	// Set scale of the cube
	transform.SetScale3D(FVector(cubeSize / 100));
	
	// Relative position to actor
	position = position - GetActorLocation();

	// Round the center to the right point
	position.X = FMath::RoundToFloat(position.X / cubeSize) * cubeSize;
	position.Y = FMath::RoundToFloat(position.Y / cubeSize) * cubeSize;
	position.Z = FMath::RoundToFloat(position.Z / cubeSize) * cubeSize;

	// Set location in the tranform
	transform.SetLocation(position);

	// Finnaly add transform to the right HISM
	meshInstances->AddInstance(transform);
}

void AWorldChunk::BuildChunk(int x1, int y1, AWorldManager & worldManager)
{

	manager = &worldManager;

	PrepareHISMs();

	SetActorLocation(FVector(x1 * worldManager.cubeSize * worldManager.chunkSize, y1 * worldManager.cubeSize * worldManager.chunkSize, 0));


	chunkBuilder = MakeUnique<FChunkBuilder>(x1, y1, worldManager);


	thread = FRunnableThread::Create(chunkBuilder.Get(),TEXT("ChunkBuilder"));

	if (thread == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("Chunkbuilder failed"));
	}
	
	GetWorldTimerManager().SetTimer(checkChunkBuilderTimer, this, &AWorldChunk::CheckChunkBuilder, 1, false, FMath::FRand());

	this->x = x1;
	this->y = y1;
}

void AWorldChunk::CubeRemovedAt(FTransform& trans)
{
}

// Called when the game starts or when spawned
void AWorldChunk::BeginPlay()
{
	Super::BeginPlay();
	
}
