// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeHISM.h"
#include "WorldChunk.h"



void UCubeHISM::RemoveCube(int32 item)
{
	FTransform t;
	GetInstanceTransform(item, t);
	owner->CubeRemovedAt(t);
	if (damagedCubes.Contains(item)) {
		damagedCubes.Remove(item);
	}
	RemoveInstance(item);
	
}

UCubeHISM::UCubeHISM(): Super()
{
	OnComponentHit.AddDynamic(this, &UCubeHISM::OnHit);
}

void UCubeHISM::DamageCube(float damage, int32 item)
{
	if (damagedCubes.Contains(item)) {
		damagedCubes[item] -= damage;
		if (damagedCubes[item] < 0) {
			RemoveCube(item);

		}
	}
	else if(cubeMaxHealth - damage < 0){
		RemoveCube(item);
	}
	else {
		damagedCubes.Add(item, cubeMaxHealth - damage);
	}
}

void UCubeHISM::HealCube(int32 item)
{
	if (damagedCubes.Contains(item)) {
		damagedCubes.Remove(item);
	}
}

void UCubeHISM::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}

void UCubeHISM::BeginPlay()
{
	owner = StaticCast<AWorldChunk*>(GetOwner());
}
