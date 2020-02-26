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
		meshInstances->RegisterComponent();
		meshInstances->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		meshInstances->SetCollisionProfileName(TEXT("Pawn"));
		meshInstances->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		meshHISMs.Add(it.name, meshInstances);
		meshInstances->SetStaticMesh(it.cubeMesh);
		meshInstances->cubeMaxHealth = it.maxHealth;
	}
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
		columnMatrix = chunkBuilder->columnMatrix;

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

void AWorldChunk::AddCube(FVector& position, FCubeType& type)
{
	// Find right hism component for this type
	UCubeHISM* meshInstances = *meshHISMs.Find(type.name);

	// Transform that will be added to the hism
	FTransform transform;

	// Set scale of the cube
	transform.SetScale3D(FVector(manager->cubeSize / 99));
	
	// Relative position to actor
	position = position - GetActorLocation();

	// Set location in the tranform
	transform.SetLocation(position);

	// Finnaly add transform to the right HISM
	meshInstances->AddInstance(transform);
}

void AWorldChunk::AddCubePrecisePosition(FVector const& position, FCubeType& type)
{
	// Find right hism component for this type
	UCubeHISM* meshInstances = *meshHISMs.Find(type.name);

	// Transform that will be added to the hism
	FTransform transform;

	// Set scale of the cube
	transform.SetScale3D(FVector(manager->cubeSize / 99));

	// Set location in the tranform
	transform.SetLocation(position);

	// Finnaly add transform to the right HISM
	meshInstances->AddInstance(transform);
}

void AWorldChunk::BuildChunk(int x1, int y1, AWorldManager & worldManager)
{

	manager = &worldManager;

	SetActorLocation(FVector(x1 * worldManager.cubeSize * worldManager.chunkSize, y1 * worldManager.cubeSize * worldManager.chunkSize, 0));

	chunkBuilder = MakeUnique<FChunkBuilder>(x1, y1 ,worldManager);

	thread = FRunnableThread::Create(chunkBuilder.Get(),TEXT("ChunkBuilder"));

	PrepareHISMs();

	if (thread == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("Chunkbuilder failed"));
	}
	
	GetWorldTimerManager().SetTimer(checkChunkBuilderTimer, this, &AWorldChunk::CheckChunkBuilder, 1, false, FMath::FRand());

	this->x = x1;
	this->y = y1;
}

void AWorldChunk::CubeRemovedAt(FTransform& trans)
{
	FVector removedLocation = trans.GetLocation();

	// Calculate this column
	float cubeSize = manager->cubeSize;
	float offset = (cubeSize * manager->chunkSize)/2;
	int X = FMath::RoundToInt((removedLocation.X + offset)/ cubeSize);
	int Y = FMath::RoundToInt((removedLocation.Y + offset)/ cubeSize);

	// Top cube
	FVector location = removedLocation;
	/*location.Z += cubeSize;
	AddCubeFromColumn(location, X, Y);*/

	// Bottom cube
	location.Z -= cubeSize;
	AddCubeFromColumn(location, X, Y);

	/*// Right cube
	X += 1;
	// If column is in another chunk
	if (X >= manager->chunkSize) {

	}
	location = removedLocation;
	location.X += cubeSize;
	AddCubeFromColumn(location, X, Y);

	// Left cube
	X -= 2;
	// If column is in another chunk
	if (X < 0) {

	}
	location = removedLocation;
	location.X -= cubeSize;
	AddCubeFromColumn(location, X, Y);
	
	X += 1;

	// Front cube
	Y += 1;
	// If column is in another chunk
	if (Y >= manager->chunkSize) {

	}
	location = removedLocation;
	location.Y += cubeSize;
	AddCubeFromColumn(location, X, Y);

	// Back cube
	Y -= 2;
	// If column is in another chunk
	if (Y < 0) {

	}
	location = removedLocation;
	location.Y -= cubeSize;
	AddCubeFromColumn(location, X, Y);*/


}

void AWorldChunk::AddCubeFromColumn(FVector const & location, int xCol, int yCol)
{
	FColumnTransform& column = (*columnMatrix.Get())[xCol * manager->chunkSize + yCol];

	if (column.topLocation.Z > location.Z + 0.0001) {
		int zDelta = FMath::RoundToInt((column.topLocation.Z - location.Z) / manager->cubeSize);

		if (zDelta < 0 || zDelta > column.types.Num() - 1) {
			return;
		}

		int & typeIndex = column.types[zDelta];

		UE_LOG(LogTemp, Warning, TEXT("Type is %d "), typeIndex);
		if (typeIndex >= 0) {
			AddCubePrecisePosition(location, manager->types[typeIndex]);
			typeIndex = -1;
		}
	}
}


// Called when the game starts or when spawned
void AWorldChunk::BeginPlay()
{
	Super::BeginPlay();
	
}
