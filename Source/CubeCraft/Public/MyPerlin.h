// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
/**
 * Custom Perlin class, mostly the same as in the engine but with added functionality
 */
class CUBECRAFT_API MyPerlin
{
public:

	/* set values used by modified perlin function*/
	static void SetOctaves(int oc);

	static void SetPersistance(float per);

	/* set values used by modified 3D perlin function*/
	static void SetOctaves3D(int oc);

	static void SetPersistance3D(float per);

	static void SetZDivisor(float zdiv);

	/**
	* Resets the random permutation used by perlin noise generation functions,
	* use RandInit() before to get some specific perlin noise .
	* @return	none
	*/
	static void PerlinReset();
	 
	 /**
	  * Generates a 1D Perlin noise from the given value.  Returns a continuous random value between -1.0 and 1.0.
	  *
	  * @param	Value	The input value that Perlin noise will be generated from.  This is usually a steadily incrementing time value.
	  *
	  * @return	Perlin noise in the range of -1.0 to 1.0
	  */
	static float PerlinNoise1D(float Value);

	/**
	* Generates a 1D Perlin noise sample at the given location.  Returns a continuous random value between -1.0 and 1.0.
	*
	* @param	Location	Where to sample
	*
	* @return	Perlin noise in the range of -1.0 to 1.0
	*/
	static float PerlinNoise2D(const FVector2D& Location);


	/**
	* Generates a 3D Perlin noise sample at the given location.  Returns a continuous random value between -1.0 and 1.0.
	*
	* @param	Location	Where to sample
	*
	* @return	Perlin noise in the range of -1.0 to 1.0
	*/
	static float PerlinNoise3D(const FVector& Location);


	/**
	* Modified perling with octaves and persistance
	*/
	static float ModifiedPerlin2D(float x, float y);


	/**
	* Modified perling with octaves and persistance
	*/
	static float ModifiedPerlin3D(float x, float y, float z);

	static float SimplexNoise3D(float x, float y, float z);
};
