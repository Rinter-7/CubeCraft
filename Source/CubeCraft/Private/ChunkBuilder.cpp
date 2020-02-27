// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkBuilder.h"
#include "WorldManager.h"
#include "MyPerlin.h"
#include "CubeHISM.h"
#include "ChunkSaver.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"



void FChunkBuilder::BuildColumn(FTransform& trans)
{

	FVector location = trans.GetLocation();

	// Fill the column
	while (location.Z > floorHeight) {
		trans.SetLocation(location);
		AddTransform(trans);
		location.Z -= cubeSize;
	}
}

void FChunkBuilder::AddTransform(FTransform const& trans)
{
	FVector location = trans.GetLocation();

	float type = MyPerlin::ModifiedPerlin3D(location.X + chunkLocation.X,
		location.Y + chunkLocation.Y,
		location.Z);

	//UE_LOG(LogTemp, Warning, TEXT("Type is %f"), type);

	// Place 4 different types into the world
	if (type > 0.4) {
		cubeComps[0]->AddInstance(trans);
	}
	else if (type > 0.1) {
		cubeComps[1]->AddInstance(trans);

	}
	else if (type > -0.8) {
		cubeComps[2]->AddInstance(trans);

	}
	else {
		cubeComps[3]->AddInstance(trans);
	}
}

void FChunkBuilder::PrepareComponents()
{
	for (int i = 0; i < nTypes; ++i) {
		cubeComps.Push(NewObject<UCubeHISM>(ownA));
		cubeComps[i]->SetCollisionProfileName(TEXT("Pawn"));
		//cubeCompsGetData(i).Get()
		cubeComps[i]->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		
	}

}

bool FChunkBuilder::Init()
{
	PrepareComponents();
	return true;
}

uint32 FChunkBuilder::Run()
{
	FString name = FString::Printf(TEXT("ChunkX_%d_Y_%d"), x, y);
	FTransform transform;
	transform.SetScale3D(FVector(cubeSize / 99));

	// Try to load chunk
	if (UChunkSaver* loadedChunk = Cast<UChunkSaver>(UGameplayStatics::LoadGameFromSlot(name, 0)))
	{
		for (int i = 0; i < loadedChunk->locations.Num(); ++i) {
			transform.SetLocation(loadedChunk->locations[i]);
			cubeComps[0]->AddInstance(transform);
		}
	}
	else {


		for (int i = 0; i < chunkSize; ++i) {
			for (int k = 0; k < chunkSize; ++k) {

				if (bIsFinished)
					return -1;

				float height = MyPerlin::ModifiedPerlin2D
				(i * cubeSize - halfOffset + chunkLocation.X,
					k * cubeSize - halfOffset + chunkLocation.Y)
					* (heightAmplitude);

				height = FMath::RoundToFloat(height / cubeSize) * cubeSize;

				transform.SetLocation(FVector(i * cubeSize - halfOffset, k * cubeSize - halfOffset, height));

				BuildColumn(transform);
			}
		}
	}
	bIsFinished = true;
	
	return 0;
}

void FChunkBuilder::Stop()
{
	bIsFinished = true;
}

bool FChunkBuilder::IsFinished()
{
	return bIsFinished;
}

FChunkBuilder::FChunkBuilder(int x1, int y1, AWorldManager & manager, AWorldChunk * owner)
{
	x = x1;
	y = y1;

	cubeSize = manager.cubeSize;
	chunkSize = manager.chunkSize;

	heightAmplitude = manager.heightAmplitude;

	halfOffset = (cubeSize * chunkSize) * 0.5;

	floorHeight = manager.floorHeight;

	nTypes = manager.types.Num();

	ownA = owner;
	chunkLocation = FVector(x * cubeSize * chunkSize, y * cubeSize * chunkSize, 0);
}
