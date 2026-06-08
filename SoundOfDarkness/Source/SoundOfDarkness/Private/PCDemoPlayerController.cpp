#include "PCDemoPlayerController.h"
#include "GameFramework/InputSettings.h"
#include "Camera/CameraComponent.h"
#include "ThrowableStone.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

APCDemoPlayerController::APCDemoPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = false;
}

void APCDemoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Inject dynamic input configurations for WASD + Mouse
	SetupDynamicInputSettings();

	// Enable player inputs
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void APCDemoPlayerController::SetupDynamicInputSettings()
{
	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	if (InputSettings)
	{
		// Force add the required keyboard/mouse mapping entries to ensure PC simulation works
		InputSettings->AddAxisMapping(FInputAxisKeyMapping("PC_MoveForward", EKeys::W, 1.f));
		InputSettings->AddAxisMapping(FInputAxisKeyMapping("PC_MoveForward", EKeys::S, -1.f));
		InputSettings->AddAxisMapping(FInputAxisKeyMapping("PC_MoveRight", EKeys::D, 1.f));
		InputSettings->AddAxisMapping(FInputAxisKeyMapping("PC_MoveRight", EKeys::A, -1.f));
		InputSettings->AddAxisMapping(FInputAxisKeyMapping("PC_Turn", EKeys::MouseX, 1.f));
		InputSettings->AddAxisMapping(FInputAxisKeyMapping("PC_LookUp", EKeys::MouseY, -1.f));

		InputSettings->AddActionMapping(FInputActionKeyMapping("PC_InteractOrThrow", EKeys::LeftMouseButton));
		InputSettings->AddActionMapping(FInputActionKeyMapping("PC_InteractOrThrow", EKeys::F));

		InputSettings->ForceRebuildKeymaps();
	}
}

void APCDemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindAxis("PC_MoveForward", this, &APCDemoPlayerController::MoveForward);
		InputComponent->BindAxis("PC_MoveRight", this, &APCDemoPlayerController::MoveRight);
		InputComponent->BindAxis("PC_Turn", this, &APCDemoPlayerController::Turn);
		InputComponent->BindAxis("PC_LookUp", this, &APCDemoPlayerController::LookUp);

		InputComponent->BindAction("PC_InteractOrThrow", IE_Pressed, this, &APCDemoPlayerController::InteractOrThrow);
	}
}

void APCDemoPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		// Force pawn rotation Yaw to match the control rotation so that forward/right movements are updated correctly
		FRotator TargetRot = ControlledPawn->GetActorRotation();
		TargetRot.Yaw = GetControlRotation().Yaw;
		ControlledPawn->SetActorRotation(TargetRot);

		// Synchronize the camera Pitch component to mouse look rotation, bypassing typical VR tracking limits in editor
		UCameraComponent* CameraComp = ControlledPawn->FindComponentByClass<UCameraComponent>();
		if (CameraComp)
		{
			FRotator CamRot = CameraComp->GetRelativeRotation();
			CamRot.Pitch = GetControlRotation().Pitch;
			CameraComp->SetRelativeRotation(CamRot);
		}
	}
}

void APCDemoPlayerController::MoveForward(float Value)
{
	if (Value == 0.0f) return;
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		FVector Direction = ControlledPawn->GetActorForwardVector();
		Direction.Z = 0.0f;
		Direction.Normalize();

		ACharacter* ControlledChar = Cast<ACharacter>(ControlledPawn);
		if (ControlledChar)
		{
			ControlledChar->AddMovementInput(Direction, Value);
		}
		else
		{
			// Fallback movement for non-character pawns (such as VRPawn)
			float DeltaTime = GetWorld()->GetDeltaSeconds();
			ControlledPawn->AddActorWorldOffset(Direction * Value * MoveSpeed * DeltaTime, true);
		}
	}
}

void APCDemoPlayerController::MoveRight(float Value)
{
	if (Value == 0.0f) return;
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		FVector Direction = ControlledPawn->GetActorRightVector();
		Direction.Z = 0.0f;
		Direction.Normalize();

		ACharacter* ControlledChar = Cast<ACharacter>(ControlledPawn);
		if (ControlledChar)
		{
			ControlledChar->AddMovementInput(Direction, Value);
		}
		else
		{
			// Fallback movement for non-character pawns
			float DeltaTime = GetWorld()->GetDeltaSeconds();
			ControlledPawn->AddActorWorldOffset(Direction * Value * MoveSpeed * DeltaTime, true);
		}
	}
}

void APCDemoPlayerController::Turn(float Value)
{
	AddYawInput(Value * MouseSensitivity);
}

void APCDemoPlayerController::LookUp(float Value)
{
	AddPitchInput(Value * MouseSensitivity);
}

void APCDemoPlayerController::InteractOrThrow()
{
	if (bIsHoldingStone)
	{
		ThrowStone();
	}
	else
	{
		TryPickupStone();
	}
}

void APCDemoPlayerController::TryPickupStone()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	FVector CameraLocation;
	FRotator CameraRotation;

	UCameraComponent* CameraComp = ControlledPawn->FindComponentByClass<UCameraComponent>();
	if (CameraComp)
	{
		CameraLocation = CameraComp->GetComponentLocation();
		CameraRotation = CameraComp->GetComponentRotation();
	}
	else
	{
		GetPlayerViewPoint(CameraLocation, CameraRotation);
	}

	FVector Start = CameraLocation;
	FVector End = Start + (CameraRotation.Vector() * GrabDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledPawn);

	// Try to locate a stone actor in front of the player
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldDynamic, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->IsA(AThrowableStone::StaticClass()))
		{
			AThrowableStone* Stone = Cast<AThrowableStone>(HitActor);
			if (Stone && Stone->StoneMesh)
			{
				HeldStone = Stone;
				bIsHoldingStone = true;

				// Disable physics behavior and collision response while holding
				Stone->StoneMesh->SetSimulatePhysics(false);
				Stone->StoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				// Attach the stone relative to camera viewport
				if (CameraComp)
				{
					Stone->AttachToComponent(CameraComp, FAttachmentTransformRules::KeepWorldTransform);
				}
				else
				{
					Stone->AttachToActor(ControlledPawn, FAttachmentTransformRules::KeepWorldTransform);
				}

				// Position the stone in the lower-right viewpoint quadrant so it's visible as "held"
				FVector HandOffset = FVector(70.0f, 25.0f, -25.0f);
				Stone->SetActorRelativeLocation(HandOffset);
				Stone->SetActorRelativeRotation(FRotator::ZeroRotator);

				UE_LOG(LogTemp, Log, TEXT("Picked up stone successfully via PC interaction."));
			}
		}
	}
}

void APCDemoPlayerController::ThrowStone()
{
	if (!HeldStone || !HeldStone->StoneMesh)
	{
		bIsHoldingStone = false;
		HeldStone = nullptr;
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	FVector CameraLocation;
	FRotator CameraRotation;

	UCameraComponent* CameraComp = ControlledPawn->FindComponentByClass<UCameraComponent>();
	if (CameraComp)
	{
		CameraLocation = CameraComp->GetComponentLocation();
		CameraRotation = CameraComp->GetComponentRotation();
	}
	else
	{
		GetPlayerViewPoint(CameraLocation, CameraRotation);
	}

	// Detach stone and reactivate physical attributes
	HeldStone->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	HeldStone->StoneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeldStone->StoneMesh->SetSimulatePhysics(true);

	// Calculate throwing path vector
	FVector LaunchDir = CameraRotation.Vector();
	FVector LaunchVelocity = LaunchDir * ThrowForce;

	// Teleport slightly forward to avoid clips/overlapping player body
	HeldStone->SetActorLocation(CameraLocation + LaunchDir * 60.0f);
	HeldStone->StoneMesh->SetPhysicsLinearVelocity(LaunchVelocity);

	UE_LOG(LogTemp, Log, TEXT("Threw stone forward via PC interaction."));

	HeldStone = nullptr;
	bIsHoldingStone = false;
}
