// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeHISM.h"
#include "WorldChunk.h"



void UCubeHISM::RemoveCube(int32 item)
{
	if (damagedCubes.Contains(item)) {
		damagedCubes.Remove(item);
	}
	RemoveInstance(item);
	
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
