// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HAL/Runnable.h"
#include "CoreMinimal.h"
#include "ColumnTransform.h"
#include "CubeType.h"
/**
 * This class will build chunk in a separate thread
 */
class CUBECRAFT_API FChunkBuilder : public FRunnable
{

	// Builds a column starting at trans
	void BuildColumn(FTransform& trans);

	// Computes 3D perlin for transform and adds it to transforms
	void AddTransform( FTransform const& trans);

	// Prepare cubehism components
	void PrepareComponents();

	// Bool that marks if we are finished
	bool bIsFinished = false;
	
	// Data copied from worldManager, they are needed for chunk generation
	int nTypes;

	int x;
	int y;

	float cubeSize;
	int chunkSize;

	float heightAmplitude;

	float halfOffset;

	float floorHeight;

	FVector chunkLocation;

	class AWorldChunk* compOwner;

	FString worldName;
public:

	/** The Data **/
	UPROPERTY()
	TArray<class UCubeHISM*> cubeComps;

	// Begin FRunnable interface.
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End FRunnable interface

	// Function that returns true if the thread has finished
	bool IsFinished();

	// Constructor
	FChunkBuilder(int x, int y, class AWorldManager & manager, class AWorldChunk * owner);
};
