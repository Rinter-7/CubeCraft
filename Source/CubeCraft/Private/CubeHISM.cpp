// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeHISM.h"



UCubeHISM::UCubeHISM(): Super()
{
	OnComponentHit.AddDynamic(this, &UCubeHISM::OnHit);
}

void UCubeHISM::DestroyCube(float damage, int32 item)
{
	if (damagedCubes.Contains(item)) {
		damagedCubes[item] -= damage;
		if (damagedCubes[item] < 0) {
			RemoveInstance(item);
			damagedCubes.Remove(item);
		}
	}
	else if(cubeMaxHealth - damage < 0){
		RemoveInstance(item);
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
