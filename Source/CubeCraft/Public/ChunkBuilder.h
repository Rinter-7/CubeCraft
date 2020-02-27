// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HAL/Runnable.h"
#include "CoreMinimal.h"
#include "ColumnTransform.h"
#include "CubeType.h"
/**
 * 
 */
class CUBECRAFT_API FChunkBuilder : public FRunnable
{
	FVector chunkLocation;

	// Builds a column starting at trans
	void BuildColumn(FTransform& trans);

	// Computes 3D perlin for transform and adds it to transforms
	void AddTransform( FTransform const& trans);

	void PrepareComponents();

	bool bIsFinished = false;

	int nTypes;

	int x;
	int y;

	float cubeSize;
	int chunkSize;

	float heightAmplitude;

	float halfOffset;

	float floorHeight;

	class AWorldChunk* ownA;

public:

	/** The Data **/
	UPROPERTY()
	TArray<class UCubeHISM*> cubeComps;

	// Begin FRunnable interface.
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End FRunnable interface

	bool IsFinished();

	FChunkBuilder(int x, int y, class AWorldManager & manager, class AWorldChunk * owner);
};
