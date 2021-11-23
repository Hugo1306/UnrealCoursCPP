// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPS2Character.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "TPS2GameMode.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// ATPS2Character

ATPS2Character::ATPS2Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a item position
	ItemPosition = CreateDefaultSubobject<USceneComponent>(TEXT("ItemPosition"));
	ItemPosition->SetupAttachment(RootComponent);
	ItemPosition->SetRelativeLocation(FVector(130, 0, 10));


	/*static ConstructorHelpers::FObjectFinder<AProjectile> Project(TEXT("AProjectile'/C++ Classes/TPS2/Projectile.h'"));

	if (Project.Succeeded())
		Projectile = Project.Object;*/

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATPS2Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATPS2Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATPS2Character::MoveRight);

	PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &ATPS2Character::PickUpObject);
	PlayerInputComponent->BindAction("ShootProjectile", IE_Pressed, this, &ATPS2Character::ShootProjectile);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ATPS2Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ATPS2Character::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ATPS2Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ATPS2Character::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ATPS2Character::OnResetVR);
}


void ATPS2Character::OnResetVR()
{
	// If TPS2 is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in TPS2.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ATPS2Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ATPS2Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ATPS2Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATPS2Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATPS2Character::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATPS2Character::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ATPS2Character::PickUpObject() 
{
	if (!isPicked) 
	{
		FHitResult Hit(ForceInit);
		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Trace")), false, this);

		bool ValueTrace = DoTrace(&Hit, &TraceParams);

		if (ValueTrace && Hit.GetActor()->ActorHasTag(FName(TEXT("PUO"))))
		{
			PickedUpObject = Hit.GetActor();

			FAttachmentTransformRules ATR = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);

			//PickedUpObject->AttachToComponent(ItemPosition, ATR, NAME_None);
			PickedUpObject->GetRootComponent()->AttachToComponent(ItemPosition, ATR, NAME_None);

			// Simulate Physics
			UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(PickedUpObject->GetComponentByClass(UStaticMeshComponent::StaticClass()));

			if (MeshComp == nullptr)
				return;

			MeshComp->SetSimulatePhysics(false);
			
			isPicked = true;
		}
		
	}
	else 
	{
		FDetachmentTransformRules DTR = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, true);

		PickedUpObject->GetRootComponent()->DetachFromComponent(DTR);

		// Simulate Physics
		UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(PickedUpObject->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (MeshComp == nullptr)
			return;

		MeshComp->SetSimulatePhysics(true);

		isPicked = false;
	}
}

bool ATPS2Character::DoTrace(FHitResult* Hit, FCollisionQueryParams* TraceParams) 
{
	FVector WorldLocation = GetActorLocation();
	FVector WorldRotation = GetActorRotation().Vector();

	FVector End = WorldLocation + (WorldRotation * 1500);

	TraceParams->bTraceComplex = true;
	TraceParams->bReturnPhysicalMaterial = true;

	bool DidTrace = GetWorld()->LineTraceSingleByObjectType(*Hit, WorldLocation, End, ECC_PhysicsBody, *TraceParams);

	return DidTrace;
}

void ATPS2Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector WorldLocation = GetActorLocation();
	FVector WorldRotation = GetActorRotation().Vector();

	FVector End = WorldLocation + (WorldRotation * 1500);

	DrawDebugLine(GetWorld(), WorldLocation, End, FColor::Red, false, 0, 0, 2);


	if (Health <= 0) 
	{
		/*ATPS2GameMode* GameMode = Cast<ATPS2GameMode>(GetWorld()->GetAuthGameMode());

		if (GameMode == nullptr)
			return;

		GameMode->ResetLevel();*/

		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
		
	}

}

void ATPS2Character::ShootProjectile() 
{
	FVector SpawnLocation = GetControlRotation().Vector() * 100 + GetActorLocation();

	GetWorld()->SpawnActor<AProjectile>(Projectile, SpawnLocation, GetControlRotation());
}
