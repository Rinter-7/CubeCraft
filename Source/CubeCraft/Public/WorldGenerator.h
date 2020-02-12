// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGenerator.generated.h"

UCLASS()
class CUBECRAFT_API AWorldGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldGenerator();

	

protected:
	// Called when actor is placed in the scene
	virtual void OnConstruction(const FTransform& Transform) override;

	// Properties that define starting size of the world map
	UPROPERTY(EditAnywhere)
		INT worldWidth = 1000, worldLength = 1000;

	// Use if you want to replicate one particular world
	UPROPERTY(EditAnywhere)
		bool bUseCustomSeed = false;

	// Seed used by random function used in world generation
	UPROPERTY(EditAnywhere, , meta = (EditCondition = "bUseCustomSeed"))
		UINT seed;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
