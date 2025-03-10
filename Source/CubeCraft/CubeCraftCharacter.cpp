// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CubeCraftCharacter.h"
#include "CubeCraftProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CubeType.h"
#include "CubeCraft/Public/WorldManager.h"
#include "CubeHISM.h"
#include "Components/StaticMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ACubeCraftCharacter

ACubeCraftCharacter::ACubeCraftCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Set our beam length
	BeamLength = 2000;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);
	FP_Gun->SetCollisionObjectType(ECollisionChannel::ECC_EngineTraceChannel2);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	spawnGuideMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnGuideMesh"));
	spawnGuideMesh->SetHiddenInGame(true);

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void ACubeCraftCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	world = GetWorld();

	// Find world manager
	TArray<AActor*> managers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldManager::StaticClass(), managers);
	worldManager = StaticCast<AWorldManager*>(managers[0]);

	spawnGuideMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	spawnGuideMesh->SetUsingAbsoluteRotation(true);
	spawnGuideMesh->SetWorldScale3D(FVector(worldManager->cubeSize/99));

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACubeCraftCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACubeCraftCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACubeCraftCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACubeCraftCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACubeCraftCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACubeCraftCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACubeCraftCharacter::LookUpAtRate);
	// Custom input axis event, beam
	beamFunction = &ACubeCraftCharacter::OnBeamDestroy;
	PlayerInputComponent->BindAxis("Beam", this, &ACubeCraftCharacter::OnBeam);
	PlayerInputComponent->BindAction("ChangeWeaponMode", IE_Pressed, this, &ACubeCraftCharacter::OnWeaponModeChange);
	PlayerInputComponent->BindAction("IncrementCubeIndex", IE_Pressed, this, &ACubeCraftCharacter::IncrementCubeType);
	PlayerInputComponent->BindAction("DecrementCubeIndex", IE_Pressed, this, &ACubeCraftCharacter::DecrementCubeType);

	PlayerInputComponent->BindAction("Save", IE_Pressed, this, &ACubeCraftCharacter::SaveWorld);
}

void ACubeCraftCharacter::SaveWorld()
{
	worldManager->SaveWorld();
}

void ACubeCraftCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<ACubeCraftProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<ACubeCraftProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ACubeCraftCharacter::OnBeam(float Value)
{
	((*this).*(beamFunction))(Value);
}

void ACubeCraftCharacter::OnBeamSpawn(float Value)
{
	timeSinceLastShot += world->GetDeltaSeconds();

	FVector start = FirstPersonCameraComponent->GetComponentLocation();
	FVector end = start + FirstPersonCameraComponent->GetForwardVector() * BeamLength;
	FHitResult hit;

	if (world->LineTraceSingleByObjectType(hit, start, end, FCollisionObjectQueryParams::AllStaticObjects)) {
		DrawDebugLine(world, FP_MuzzleLocation->GetComponentLocation(), hit.Location, FColor::Green);

		// Offset the location a bit so rounding to center of the potential cube will work
		FVector location = hit.Location + hit.Normal;

		// Cache cubesize for further use
		float cubeSize = worldManager->cubeSize;

		// Round the center to the right point
		location.X = FMath::RoundToFloat(location.X / cubeSize) * cubeSize;
		location.Y = FMath::RoundToFloat(location.Y / cubeSize) * cubeSize;
		location.Z = FMath::RoundToFloat(location.Z / cubeSize) * cubeSize;

		spawnGuideMesh->SetHiddenInGame(false);
		spawnGuideMesh->SetWorldLocation(location);

		if (Value > 0.1 && timeSinceLastShot >= reloadTime && hit.Distance > 200) {
			worldManager->AddCube(location, spawnTypeIndex);
			timeSinceLastShot = 0;
		}
	}
	else {
		spawnGuideMesh->SetHiddenInGame(true);
		DrawDebugLine(world, FP_MuzzleLocation->GetComponentLocation(), end, FColor::Blue);
	}
}

void ACubeCraftCharacter::OnBeamDestroy(float Value)
{
	if (Value > 0.1) {
		FVector start = FirstPersonCameraComponent->GetComponentLocation();
		FVector end = start + FirstPersonCameraComponent->GetForwardVector() * BeamLength;
		FHitResult hit;
		if (world->LineTraceSingleByObjectType(hit, start, end, FCollisionObjectQueryParams::AllStaticObjects)) {
			DrawDebugLine(world, FP_MuzzleLocation->GetComponentLocation(), hit.Location, FColor::Red);

			if (hit.GetComponent() == NULL)
				return;
			
			if (damagedComponent == hit.GetComponent() && damagedCube == hit.Item) {
				damagedComponent->DamageCube(damage * world->GetDeltaSeconds(), damagedCube);
			}
			else {
				if (damagedComponent != NULL) 
					damagedComponent->HealCube(damagedCube);
				
				damagedComponent = dynamic_cast<UCubeHISM*>(hit.GetComponent());
				if (damagedComponent == NULL)
					return;

				damagedCube = hit.Item;
				damagedComponent->DamageCube(damage * world->GetDeltaSeconds(), damagedCube);
			}
		}
		else{
			DrawDebugLine(world, FP_MuzzleLocation->GetComponentLocation(), end, FColor::Blue);
			if (damagedComponent != NULL) {
				damagedComponent->HealCube(damagedCube);
				damagedComponent = NULL;
			}
		}
	}
	else if(damagedComponent != NULL) {
		damagedComponent->HealCube(damagedCube);
		damagedComponent = NULL;
	}
}


void ACubeCraftCharacter::OnWeaponModeChange()
{
	bDestructionMode ^= true;

	if (bDestructionMode) {
		spawnGuideMesh->SetHiddenInGame(true);
		beamFunction = &ACubeCraftCharacter::OnBeamDestroy;
	}
	else {
		if (damagedComponent != NULL) {
			damagedComponent->HealCube(damagedCube);
			damagedComponent = NULL;
		}
		beamFunction = &ACubeCraftCharacter::OnBeamSpawn;
	}
}

void ACubeCraftCharacter::IncrementCubeType()
{
	if (++spawnTypeIndex >= worldManager->types.Num()) {
		spawnTypeIndex = 0;
	}
}

void ACubeCraftCharacter::DecrementCubeType()
{
	if (--spawnTypeIndex < 0) {
		spawnTypeIndex = worldManager->types.Num() - 1;
	}
}

void ACubeCraftCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACubeCraftCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ACubeCraftCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void ACubeCraftCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void ACubeCraftCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACubeCraftCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ACubeCraftCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACubeCraftCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool ACubeCraftCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ACubeCraftCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ACubeCraftCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ACubeCraftCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}
