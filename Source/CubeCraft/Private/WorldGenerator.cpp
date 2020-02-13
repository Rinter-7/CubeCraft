// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"




float AWorldGenerator::ModifiedPerlin(float x, float y)
{
	// Normalize input to be in range 0 - 1
	x /= 100000;
	y /= 100000;

	float total = 0;
	float frequency = 1;
	float amplitude = 1;
	float maxValue = 0;  // Used for normalizing result to 0.0 - 1.0
	for (int i = 0; i < octaves; i++) {
		total += FMath::PerlinNoise2D(FVector2D(x * frequency, y * frequency)) * amplitude;

		maxValue += amplitude;

		amplitude *= persistance;
		frequency *= 2;
	}

	return total / maxValue;
}

void AWorldGenerator::InitializeSeedArray(int size, float * seedArr)
{
	FMath::RandInit(seed);

	for (int i = 0; i < size; ++i)
		seedArr[i] = FMath::Rand() / (float)RAND_MAX;
}

// Sets default values
AWorldGenerator::AWorldGenerator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Geometry/Meshes/Cube.Cube'")).Object;

	meshInstances = ObjectInitializer.CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(this, TEXT("meshInstances"));

	//meshInstances->SetFlags(RF_Transactional);
	meshInstances->AttachTo(RootComponent);

	meshInstances->SetStaticMesh(cubeMesh);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AWorldGenerator::GenerateWorld()
{
	if (!bUseCustomSeed)
		seed = FDateTime::Now().ToUnixTimestamp();



	

	for (int x = 0; x < worldWidth; ++x) {
		for (int y = 0; y < worldLength; ++y) {

			float height = ModifiedPerlin(x,y) * heightAmplitude;
			height = FMath::RoundToFloat(height / pieceSize) * pieceSize;

			FTransform transform;
			transform.SetLocation(FVector(x * pieceSize - worldWidth/2, y * pieceSize - worldLength/2, height));
			transform.SetScale3D(FVector(pieceSize/100));
			meshInstances->AddInstance(transform);

			transform.SetLocation(FVector(x * pieceSize - worldWidth / 2, y * pieceSize - worldLength / 2, height - pieceSize ));
			meshInstances->AddInstance(transform);


			//AStaticMeshActor* instance = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(x * pieceSize, y * pieceSize, height), FRotator::ZeroRotator);
			//instance->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
			//instance->SetActorScale3D(FVector(pieceSize/100));
			//cubes.Add(instance);
		}
	}

}

void AWorldGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!isGenerated) {
		FMath::RandInit(seed);
		/*for (auto&& it : cubes) {
			it->Destroy();
		}
		cubes.Empty();*/
		meshInstances->ClearInstances();
		meshInstances->SetStaticMesh(cubeMesh);

		GenerateWorld();
		isGenerated = true;
	}
}

void AWorldGenerator::OnConstruction(const FTransform& Transform)
{
	//meshInstances->RegisterComponent();

}

// Called when the game starts or when spawned
void AWorldGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWorldGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

