// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "CubeCraft/Public/WorldChunk.h"
#include "CubeCraft/CubeCraftCharacter.h"
#include "CubeCraft/Public/MyPerlin.h"
#include "Kismet/GameplayStatics.h"
#include "WorldSaver.h"


void AWorldManager::ResetPlayerPosition()
{
	GetWorld()->GetFirstPlayerController()->GetPawn()->SetActorLocation(playerPosition);
}

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

	removedChunks.Push(FIntPoint(x,y));

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


void AWorldManager::SaveWorld()
{
	ShouldDestroy = false;
	TArray<AWorldChunk*> chunks;
	quadTree->GetElements(quadTree->GetTreeBox(), chunks);
	for (auto&& it : chunks) {
		it->SaveChunk();
	}

	if (UWorldSaver* savedWorld = Cast<UWorldSaver>(UGameplayStatics::CreateSaveGameObject(UWorldSaver::StaticClass())))
	{
		savedWorld->seed = seed;
		savedWorld->centerX = centerX;
		savedWorld->centerY = centerY;
		savedWorld->playerLocation = playerController->GetFocalLocation();

		UGameplayStatics::SaveGameToSlot(savedWorld,worldName + "\\world", 0);
	}
}

void AWorldManager::LoadWorld()
{
	if (UWorldSaver* savedWorld = Cast<UWorldSaver>(UGameplayStatics::LoadGameFromSlot(worldName + "\\world", 0)))
	{
		seed = savedWorld->seed;
		centerX = savedWorld->centerX;
		centerY = savedWorld->centerY;
		playerPosition = savedWorld->playerLocation;
		GetWorld()->GetFirstPlayerController()->GetPawn()->SetActorLocation(playerPosition);
		FTimerHandle tmp;
		GetWorld()->GetTimerManager().SetTimer(tmp, this, &AWorldManager::ResetPlayerPosition, 5, false);
	}
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
}

void AWorldManager::RecenterWorld(int newCenterX, int newCenterY)
{
	// Center is the same
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

void AWorldManager::AddCube(FVector& position, int typeIndex)
{
	// Get chunk coordinates
	int x = FMath::RoundToInt(position.X / chunkLength);
	int y = FMath::RoundToInt(position.Y / chunkLength);
	
	TArray<AWorldChunk*> chunks;
	quadTree->GetElements(BoxAroundPoint(x, y, 0.2f), chunks);

	if (chunks.Num() > 0) {
		chunks[0]->AddCube(position, typeIndex);
	}
}



// Called when the game starts or when spawned
void AWorldManager::BeginPlay()
{
	Super::BeginPlay();
	
	LoadWorld();

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

	// Start timer function that checks player position
	GetWorldTimerManager().SetTimer(checkPlayerPositionTimerHandle, this, &AWorldManager::CheckPlayerPosition, 1.0f, true, 1.f);

	// Build starting chunks of the world
	for (int x = -nVisibleChunks + 1 + centerX; x < nVisibleChunks + centerX; ++x) {
		for (int y = -nVisibleChunks + 1 + centerY; y < nVisibleChunks + centerY; ++y) {
			AddChunk(x, y);
		}
	}
}

void AWorldManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ShouldDestroy)
	{
		for (auto&& it : removedChunks) {
			FString name = worldName + FString::Printf(TEXT("\\ChunkX_%d_Y_%d"), it.X, it.Y);
			UGameplayStatics::DeleteGameInSlot(name, 0);
		}
	}
}
