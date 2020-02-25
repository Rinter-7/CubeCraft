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




void AWorldChunk::PrepareHISMs()
{
	for (auto&& it : manager->types) {
		UCubeHISM* meshInstances = NewObject< UCubeHISM>(this);
		PrepareHISM(meshInstances);
		meshHISMs.Add(it.name, meshInstances);
		meshInstances->SetStaticMesh(it.cubeMesh);
	}
}

void AWorldChunk::PrepareHISM(UCubeHISM* hism)
{
	hism->RegisterComponent();
	hism->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	hism->SetCollisionProfileName(TEXT("Pawn"));
	hism->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
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
	owningTree->Remove(this, FBox2D(FVector2D(x - 0.1, y - 0.1), FVector2D(x + 0.1, y + 0.1)));
	this->Destroy();
}

void AWorldChunk::AddMeshInstanceToChunk(UStaticMesh& staticMesh, FTransform& transform)
{
}

void AWorldChunk::AddCube(FVector& position, FCubeType& type)
{
	// Find right hism component for this type
	UHierarchicalInstancedStaticMeshComponent* meshInstances = *meshHISMs.Find(type.name);

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

	owningTree = worldManager.quadTree;

	manager = &worldManager;

	this->x = x1;
	this->y = y1;
	float cubeSize = worldManager.cubeSize;
	int chunkSize = worldManager.chunkSize;

	SetActorLocation(FVector(x1* cubeSize * chunkSize, y1* cubeSize * chunkSize, 0));

	PrepareHISMs();
	UHierarchicalInstancedStaticMeshComponent* dirtInstances = *meshHISMs.Find("dirt");
	UHierarchicalInstancedStaticMeshComponent* stoneInstances = *meshHISMs.Find("stone");
	UHierarchicalInstancedStaticMeshComponent* diamondInstances = *meshHISMs.Find("diamond");
	UHierarchicalInstancedStaticMeshComponent* iceInstances = *meshHISMs.Find("ice");

	FTransform transform;
	transform.SetScale3D(FVector(cubeSize/100));
	float halfOffset = (cubeSize * chunkSize) * 0.5;
	for (int i = 0; i < chunkSize; ++i) {
		for (int k = 0; k < chunkSize; ++k) {
			float height = MyPerlin::ModifiedPerlin2D
				(i*cubeSize - halfOffset + GetActorLocation().X,
					k * cubeSize - halfOffset + GetActorLocation().Y)
				*(worldManager.heightAmplitude);

			height = FMath::RoundToFloat(height / cubeSize) * cubeSize;

			transform.SetLocation(FVector(i*cubeSize - halfOffset, k * cubeSize - halfOffset , height));

			float type = MyPerlin::ModifiedPerlin3D(i * cubeSize - halfOffset + GetActorLocation().X,
												k * cubeSize - halfOffset + GetActorLocation().Y,
													height);
			if (type > 0.6)
				continue;
			if(type > 0.3)
				iceInstances->AddInstance(transform);
			else if(type > 0.1)
				stoneInstances->AddInstance(transform);
			else if(type > -0.8)
				dirtInstances->AddInstance(transform);
			else 
				diamondInstances->AddInstance(transform);
		}
	}
}

// Called when the game starts or when spawned
void AWorldChunk::BeginPlay()
{
	Super::BeginPlay();
	
}
