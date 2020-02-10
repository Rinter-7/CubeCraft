// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CubeCraftHUD.generated.h"

UCLASS()
class ACubeCraftHUD : public AHUD
{
	GENERATED_BODY()

public:
	ACubeCraftHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

