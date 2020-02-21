// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldChunk.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "CubeCraft/Public/WorldManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"




void AWorldChunk::PrepareHISMs()
{
	UHierarchicalInstancedStaticMeshComponent* meshInstances = NewObject< UHierarchicalInstancedStaticMeshComponent>(this);
	meshInstances->RegisterComponent();

	meshInstances->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	meshInstances->SetStaticMesh(cubeMesh);
	meshHISMs.Add("Basic", meshInstances);
	meshInstances->SetCollisionProfileName(TEXT("Pawn"));
	meshInstances->OnComponentHit.AddDynamic(this, &AWorldChunk::OnCompHit);
	meshInstances->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
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

void AWorldChunk::DisableChunk()
{
	SetActorHiddenInGame(true);
	bIsActive = false;
	GetWorld()->GetTimerManager().SetTimer(saveAndDestroyTimerHandle, this, &AWorldChunk::SaveAndDestroy, destroyTime, false);
}

void AWorldChunk::ActivateChunk()
{
	bIsActive = true;

	if (IsHidden()) {
		SetActorHiddenInGame(false);
		return;
	}
}

void AWorldChunk::SaveAndDestroy()
{
	saveAndDestroyTimerHandle.Invalidate();
	if (bIsActive)
		return;
	//FBox2D(FVector2D(x - halfsize, y - halfsize), FVector2D(x + halfsize, y + halfsize));
	owningTree->Remove(this, FBox2D(FVector2D(x - 0.1, y - 0.1), FVector2D(x + 0.1, y + 0.1)));
	this->Destroy();
}

void AWorldChunk::AddMeshInstanceToChunk(UStaticMesh& staticMesh, FTransform& transform)
{
}

void AWorldChunk::AddCube(FVector& position, const FString& type)
{
	// Find right hism component for this type
	UHierarchicalInstancedStaticMeshComponent* meshInstances = *meshHISMs.Find(type);

	// Transform that will be added to the hism
	FTransform transform;

	// Cache cubesize for further use
	float cubeSize = manager->cubeSize;

	// Set scale of the cube
	transform.SetScale3D(FVector(cubeSize / 100));
	
	// Relative position to actor
	position = position - GetActorLocation();

	// Round the center to the right point
	position.X = FMath::RoundToFloat(position.X / cubeSize) * cubeSize;
	position.Y = FMath::RoundToFloat(position.Y / cubeSize) * cubeSize;
	position.Z = FMath::RoundToFloat(position.Z / cubeSize) * cubeSize;

	// Set location in the tranform
	transform.SetLocation(position);

	// Finnaly add transform to the right HISM
	meshInstances->AddInstance(transform);
}

void AWorldChunk::BuildChunk(int x1, int y1, AWorldManager & worldManager)
{

	PrepareHISMs();

	owningTree = worldManager.quadTree;

	manager = &worldManager;

	this->x = x1;
	this->y = y1;
	float cubeSize = worldManager.cubeSize;
	int chunkSize = worldManager.chunkSize;
	SetActorLocation(FVector(x1* cubeSize * chunkSize, y1* cubeSize * chunkSize, 0));

	UHierarchicalInstancedStaticMeshComponent* meshInstances = *meshHISMs.Find("Basic");

	FTransform transform;
	transform.SetScale3D(FVector(cubeSize/100));
	float halfOffset = (cubeSize * chunkSize) * 0.5;
	for (int i = 0; i < chunkSize; ++i) {
		for (int k = 0; k < chunkSize; ++k) {
			float height = worldManager.ModifiedPerlin
				(i*cubeSize - halfOffset + GetActorLocation().X,
					k * cubeSize - halfOffset + GetActorLocation().Y)
				*(worldManager.heightAmplitude);

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

