// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"




void AWorldGenerator::PerlinNoise2D(float * seedArr,float * outputArr)
{
	for (int x = 0; x < worldWidth; ++x)
		for(int y = 0; y < worldLength; ++y)
		{
			float noise = 0.f;
			float scale = 1.f;

			// Accumulation of scale
			float scaleAcc = 0.f;

			for (int o = 0; o < octaves; ++o) {
				int pitch = worldWidth >> o;
				int sampleX1 = (x / pitch) * pitch;
				int sampleX2 = (sampleX1 + pitch) % worldWidth;

				int sampleY1 = (y / pitch) * pitch;
				int sampleY2 = (sampleY1 + pitch) % worldWidth;

				float blendX = (float)(x - sampleX1) / (float)pitch;
				float blendY = (float)(x - sampleY1) / (float)pitch;

				float sampleA = (1.f - blendX) * seedArr[sampleY1 * worldWidth + sampleX1] + blendX * seedArr[sampleY1 * worldWidth + sampleX2];
				float sampleB = (1.f - blendX) * seedArr[sampleY2 * worldWidth + sampleX1] + blendX * seedArr[sampleY2 * worldWidth + sampleX2];

				noise += (blendY * (sampleB - sampleA) + sampleA) * scale;
				scaleAcc += scale;
				scale /= bias;
			}

			outputArr[y * worldWidth + x] = noise / scaleAcc;
		}
}

void AWorldGenerator::InitializeSeedArray(int size, float * seedArr)
{
	FMath::RandInit(seed);

	for (int i = 0; i < size; ++i)
		seedArr[i] = FMath::Rand() / (float)RAND_MAX;
}

// Sets default values
AWorldGenerator::AWorldGenerator()
{
	auto mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Geometry/Meshes/Cube.Cube'"));
	cubeMesh = mesh.Object;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AWorldGenerator::GenerateWorld()
{
	if (!bUseCustomSeed)
		seed = FDateTime::Now().ToUnixTimestamp();
	float* seedArray = new float[worldWidth * worldLength];

	InitializeSeedArray(worldWidth * worldLength, seedArray);

	float* perlinOutput = new float[worldWidth * worldLength];

	PerlinNoise2D(seedArray, perlinOutput);

	for (int x = 0; x < worldWidth; ++x) {
		for (int y = 0; y < worldLength; ++y) {

			float height = perlinOutput[y * worldWidth + x];
			

			AStaticMeshActor* instance = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(x * 100, y * 100, height * 800), FRotator::ZeroRotator);
			instance->GetStaticMeshComponent()->SetStaticMesh(cubeMesh);
			cubes.Add(instance);
		}
	}

	delete(seedArray);
	delete(perlinOutput);
}

void AWorldGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!isGenerated) {
		for (auto&& it : cubes) {
			it->Destroy();
		}
		cubes.Empty();
		GenerateWorld();
		isGenerated = true;
	}
}

void AWorldGenerator::OnConstruction(const FTransform& Transform)
{

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

