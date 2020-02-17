// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldChunk.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

void AWorldChunk::PrepareHISMs()
{
	UHierarchicalInstancedStaticMeshComponent* meshInstances = NewObject< UHierarchicalInstancedStaticMeshComponent>(this);
	meshInstances->RegisterComponent();

	meshInstances->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	meshInstances->SetStaticMesh(cubeMesh);
	meshHISMs.Add("Basic", meshInstances);
	meshInstances->SetCollisionProfileName(TEXT("Pawn"));
	meshInstances->OnComponentHit.AddDynamic(this, &AWorldChunk::OnCompHit);
}

float AWorldChunk::ModifiedPerlin(float x1, float y1)
{
	// Normalize input to be in range 0 - 1
	x1 /= -1048576;
	y1 /= -1048576;

	float total = 0;
	float frequency = 1;
	float amplitude = 1;
	float maxValue = 0;  // Used for normalizing result to 0.0 - 1.0
	for (int i = 0; i < octaves; i++) {
		total += FMath::PerlinNoise2D(FVector2D(x1 * frequency, y1 * frequency)) * amplitude;

		maxValue += amplitude;

		amplitude *= persistance;
		frequency *= 2;
	}

	return total / maxValue;
}

AWorldChunk::AWorldChunk()
{

	root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	RootComponent = root;

	cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Geometry/Meshes/Cube.Cube'")).Object;
	PrimaryActorTick.bCanEverTick = false;

}


bool AWorldChunk::LoadChunk(int x1, int y1)
{
	return false;
}

void AWorldChunk::SaveChunk()
{
}

void AWorldChunk::DestroyChunk()
{
}

void AWorldChunk::SaveAndDestroy()
{
}

void AWorldChunk::AddMeshInstanceToChunk(UStaticMesh& staticMesh, FTransform& transform)
{
}

void AWorldChunk::BuildChunk(int x1, int y1, float cubeSize1, int chunkSize1)
{
	PrepareHISMs();
	this->x = x1;
	this->y = y1;
	this->cubeSize = cubeSize1;
	this->chunkSize = chunkSize1;
	SetActorLocation(FVector(x1*cubeSize1*chunkSize1, y1*cubeSize1*chunkSize1, 0));

	UHierarchicalInstancedStaticMeshComponent* meshInstances = *meshHISMs.Find("Basic");


	FTransform transform;
	transform.SetScale3D(FVector(cubeSize/100));
	float halfOffset = (cubeSize1 * chunkSize1) * 0.5;
	for (int i = 0; i < chunkSize; ++i) {
		for (int k = 0; k < chunkSize; ++k) {
			float height = ModifiedPerlin(i*cubeSize - halfOffset + GetActorLocation().X, k * cubeSize - halfOffset + GetActorLocation().Y) * heightAmplitude;
			height = FMath::RoundToFloat(height / cubeSize) * cubeSize;

			transform.SetLocation(FVector(i*cubeSize - halfOffset, k * cubeSize - halfOffset , height));

			meshInstances->AddInstance(transform);
		}
	}
}

// Called when the game starts or when spawned
void AWorldChunk::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWorldChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWorldChunk::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherComp->GetCollisionObjectType() == ECollisionChannel::ECC_GameTraceChannel1) {
		UHierarchicalInstancedStaticMeshComponent* hitHISM = StaticCast<UHierarchicalInstancedStaticMeshComponent*>(HitComp);
		hitHISM->RemoveInstance(Hit.Item);
	}

}

