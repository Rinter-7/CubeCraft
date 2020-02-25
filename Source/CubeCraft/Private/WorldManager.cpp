// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "CubeCraft/Public/WorldChunk.h"
#include "CubeCraft/CubeCraftCharacter.h"
#include "CubeCraft/Public/MyPerlin.h"

void AWorldManager::AddChunk(int x, int y)
{
	TArray<AWorldChunk*> chunks;
	quadTree->GetElements(BoxAroundPoint(x, y, 0.2f), chunks);

	if (chunks.Num() > 0 ) {
		chunks[0]->ActivateChunk();
		return;
	}

	//Build a new chunk if its needed
	UWorld* world = GetWorld();
	AWorldChunk*chunk = world->SpawnActor<AWorldChunk>();

	quadTree->Insert(chunk,BoxAroundPoint(x,y,0.1f));
	chunk->BuildChunk(x, y, *this);
}

void AWorldManager::RemoveChunk(int x, int y)
{
	TArray<AWorldChunk*> chunks;
	quadTree->GetElements(BoxAroundPoint(x, y,0.2f), chunks);

	if (chunks.Num() > 0) {
		chunks[0]->DisableChunk();
		return;
	}
}

FBox2D AWorldManager::BoxAroundPoint(int x, int y, float size)
{
	float halfsize = size / 2;
	return FBox2D(FVector2D(x - halfsize, y - halfsize), FVector2D(x + halfsize, y + halfsize));
}


// Sets default values
AWorldManager::AWorldManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Init the random seed
	seed = FDateTime::UtcNow().ToUnixTimestamp();

	// Init quad tree
	quadTree = MakeShared< TQuadTree<AWorldChunk*>>(BoxAroundPoint(0, 0, 10000), 0.5f);

	// ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Geometry/Meshes/Cube.Cube'")).Object;
}

void AWorldManager::RecenterWorld(int newCenterX, int newCenterY)
{
	if (centerX == newCenterX && centerY == newCenterY) {
		return;
	}
	// Get all visible chunks
	TArray<AWorldChunk*> chunks;
	quadTree->GetElements(BoxAroundPoint(centerX,centerY, 2 * (nVisibleChunks - 1) + 0.2),chunks);

	for (auto&& it : chunks) {
		// If chunk is out of boundaries destroy it
		if (it->x < -nVisibleChunks + 1 + newCenterX || 
			it->x > nVisibleChunks + newCenterX - 1 ||
			it->y < -nVisibleChunks + 1 + newCenterY ||
			it->y > nVisibleChunks + newCenterY - 1)
		{
			RemoveChunk(it->x, it->y);

			// For every chunk we destroy there must be a chunk that we need to create, 
			AddChunk((it->x - centerX)*(-1) + newCenterX, (it->y - centerY) * (-1) + newCenterY);
		}
	}
	
	// Update the new center
	centerX = newCenterX;
	centerY = newCenterY;
}

void AWorldManager::CheckPlayerPosition()
{
	// If playerconotller is null, try to get it and return, this function is called repeatedly
	// so there is no need to check playerController again
	if (playerController == nullptr) {
		playerController = GetWorld()->GetFirstPlayerController();
		return;
	}

	FVector location = playerController->GetFocalLocation();

	int x = FMath::RoundToInt( location.X / chunkLength);
	int y = FMath::RoundToInt( location.Y / chunkLength);

	if (x != centerX || y != centerY) {
		RecenterWorld(x, y);
	}
}

void AWorldManager::AddCube(FVector& position, FCubeType& type)
{
	// Get chunk coordinates

	int x = FMath::RoundToInt(position.X / chunkLength);
	int y = FMath::RoundToInt(position.Y / chunkLength);
	
	TArray<AWorldChunk*> chunks;
	quadTree->GetElements(BoxAroundPoint(x, y, 0.2f), chunks);

	if (chunks.Num() > 0) {
		chunks[0]->AddCube(position, type);
	}
	else {
		FString msg;
		msg.Append("X: ");
		msg.AppendInt(x);
		msg.Append(" Y: ");
		msg.AppendInt(y);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, msg);

	}
}



// Called when the game starts or when spawned
void AWorldManager::BeginPlay()
{
	Super::BeginPlay();

	// Empty the tree if we are in the editor
	quadTree->Empty();

	MyPerlin::SetOctaves(octaves);

	MyPerlin::SetPersistance(persistance);

	MyPerlin::SetOctaves3D(octaves3D);

	MyPerlin::SetPersistance3D(persistance3D);

	MyPerlin::SetZDivisor(zDivisor);

	// Reseed random function
	FMath::RandInit(seed);

	// Reset the perlin noise with the new seed in fmath::rand
	MyPerlin::PerlinReset();

	// Calculate chunk length
	chunkLength = chunkSize * cubeSize;

	// Get player conteller
	playerController = GetWorld()->GetFirstPlayerController();

	if (playerController) {
		APawn * pawn = playerController->GetPawn();
		if (pawn) {
			StaticCast<ACubeCraftCharacter*>(pawn)->worldManager = this;
		}
	}

	// Start timer function that checks player position
	GetWorldTimerManager().SetTimer(checkPlayerPositionTimerHandle, this, &AWorldManager::CheckPlayerPosition, 1.0f, true, 1.f);

	// Set world center
	centerX = 0;
	centerY = 0;

	// Build starting chunks of the world
	for (int x = -nVisibleChunks + 1; x < nVisibleChunks; ++x) {
		for (int y = -nVisibleChunks + 1; y < nVisibleChunks; ++y) {
			AddChunk(x, y);
		}
	}
}

void AWorldManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//delete(quadTree);
}
