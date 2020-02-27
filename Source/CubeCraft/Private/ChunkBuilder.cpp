// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkBuilder.h"
#include "WorldManager.h"
#include "MyPerlin.h"
#include "CubeHISM.h"
#include "ChunkSaver.h"
#include "Kismet/GameplayStatics.h"


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
		transforms.Push(trans);
		typeIndexes.Push(0);
	}
	else if (type > 0.1) {
		transforms.Push(trans);
		typeIndexes.Push(1);
	}
	else if (type > -0.8) {
		transforms.Push(trans);
		typeIndexes.Push(2);
	}
	else {
		transforms.Push(trans);
		typeIndexes.Push(3);
	}
}

bool FChunkBuilder::Init()
{

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
			transforms.Push(transform);
		}
		typeIndexes = loadedChunk->types;
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

FChunkBuilder::FChunkBuilder(int x1, int y1, AWorldManager & manager)
{
	x = x1;
	y = y1;

	cubeSize = manager.cubeSize;
	chunkSize = manager.chunkSize;

	heightAmplitude = manager.heightAmplitude;

	halfOffset = (cubeSize * chunkSize) * 0.5;

	floorHeight = manager.floorHeight;

	chunkLocation = FVector(x * cubeSize * chunkSize, y * cubeSize * chunkSize, 0);
}
