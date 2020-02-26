// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkBuilder.h"
#include "WorldManager.h"
#include "MyPerlin.h"
#include "CubeHISM.h"


void FChunkBuilder::BuildColumnVisible(FTransform& trans)
{
	//Add top transform 
	AddTransform(trans);

	FVector location = trans.GetLocation();

	//Check for gap under this transform
	int gap = CalculateGap(trans);

	// Fill the gap
	for (int i = 0; i < gap; ++i) {
		location.Z -= cubeSize;
		trans.SetLocation(location);
		AddTransform(trans);
	}
}

int FChunkBuilder::CalculateGap(FTransform const& trans)
{
	FVector location = trans.GetLocation();
	location += chunkLocation; // To world coords

	// Heights of the neigbour cubes
	float height = FMath::RoundToFloat((MyPerlin::ModifiedPerlin2D(location.X + cubeSize, location.Y) * heightAmplitude) / cubeSize) * cubeSize;
	float height2 = FMath::RoundToFloat((MyPerlin::ModifiedPerlin2D(location.X - cubeSize, location.Y) * heightAmplitude) / cubeSize) * cubeSize;
	float height3 = FMath::RoundToFloat((MyPerlin::ModifiedPerlin2D(location.X, location.Y + cubeSize) * heightAmplitude) / cubeSize) * cubeSize;
	float height4 = FMath::RoundToFloat((MyPerlin::ModifiedPerlin2D(location.X, location.Y - cubeSize) * heightAmplitude) / cubeSize) * cubeSize;

	// Minimum Height out of the neighbours
	height = FMath::Min(FMath::Min(height,height2), FMath::Min(height3, height4));

	// If the minimum height is less then one cube under this one, there is a gap
	if (height < location.Z - cubeSize + 0.0001) {
		// Calculate the gap
		return (location.Z - height + 0.0001) / cubeSize - 1;
	}
	else {
		return 0;
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
		types.Push(0);
	}
	else if (type > 0.1) {
		transforms.Push(trans);
		types.Push(1);
	}
	else if (type > -0.8) {
		transforms.Push(trans);
		types.Push(2);
	}
	else {
		transforms.Push(trans);
		types.Push(3);
	}
}

void FChunkBuilder::BuildColumnRest(FTransform& trans, int xLoc, int yLoc)
{
	FVector location = trans.GetLocation();

	TArray<int>& typesCol = (*columnMatrix.Get())[yLoc * chunkSize + xLoc].types;

	(*columnMatrix.Get())[yLoc * chunkSize + xLoc].topLocation = location;

	while (location.Z > -1000) {
		location.Z -= cubeSize;

		float type = MyPerlin::ModifiedPerlin3D(location.X + chunkLocation.X,
			location.Y + chunkLocation.Y,
			location.Z);

		//UE_LOG(LogTemp, Warning, TEXT("Type is %f"), type);

		// Place 4 different types into the world
		if (type > 0.4) {
			typesCol.Push(0);
		}
		else if (type > 0.1) {
			typesCol.Push(1);

		}
		else if (type > -0.8) {
			typesCol.Push(2);

		}
		else {
			typesCol.Push(3);
		}
	}
}

bool FChunkBuilder::Init()
{

	return true;
}

uint32 FChunkBuilder::Run()
{
	FTransform transform;
	transform.SetScale3D(FVector(cubeSize / 99));


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

			BuildColumnVisible(transform);
			BuildColumnRest(transform, i, k);
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

	columnMatrix = MakeShared<TArray<FColumnTransform>>();

	columnMatrix->AddDefaulted(chunkSize * chunkSize);

	heightAmplitude = manager.heightAmplitude;

	halfOffset = (cubeSize * chunkSize) * 0.5;

	chunkLocation = FVector(x * cubeSize * chunkSize, y * cubeSize * chunkSize, 0);
}
