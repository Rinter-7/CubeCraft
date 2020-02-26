// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HAL/Runnable.h"
#include "CoreMinimal.h"
#include "ColumnTransform.h"
/**
 * 
 */
class CUBECRAFT_API FChunkBuilder : public FRunnable
{
	FVector chunkLocation;

	// Builds a column starting at trans
	void BuildColumnVisible(FTransform& trans);

	// Check 4 sides of the cube, if there is a gap calculate how big it is, 0 mean no gap
	int CalculateGap( FTransform const& trans);

	// Computes 3D perlin for transform and adds it to transforms
	void AddTransform( FTransform const& trans);

	void BuildColumnRest(FTransform& trans, int xLoc, int yLoc);

	bool bIsFinished = false;

	int x;
	int y;

	float cubeSize;
	int chunkSize;

	float heightAmplitude;

	float halfOffset;

public:
	/** The Data **/
	TArray<FTransform> transforms;

	TArray<int> types;

	TSharedPtr<TArray<FColumnTransform>> columnMatrix;

	// Begin FRunnable interface.
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End FRunnable interface

	bool IsFinished();


	FChunkBuilder(int x, int y, class AWorldManager & manager);

};
