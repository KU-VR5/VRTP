#include "KeyboardMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"

UKeyboardMovementComponent::UKeyboardMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UKeyboardMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UKeyboardMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	// 1. Keyboard Movement (WASD)
	float ForwardValue = 0.0f;
	if (PC->IsInputKeyDown(EKeys::W)) ForwardValue += 1.0f;
	if (PC->IsInputKeyDown(EKeys::S)) ForwardValue -= 1.0f;

	float RightValue = 0.0f;
	if (PC->IsInputKeyDown(EKeys::D)) RightValue += 1.0f;
	if (PC->IsInputKeyDown(EKeys::A)) RightValue -= 1.0f;

	if (ForwardValue != 0.0f || RightValue != 0.0f)
	{
		FVector ForwardDir = Pawn->GetActorForwardVector();
		FVector RightDir = Pawn->GetActorRightVector();

		// Keep movement strictly on the horizontal plane (XY)
		ForwardDir.Z = 0.0f;
		ForwardDir.Normalize();
		RightDir.Z = 0.0f;
		RightDir.Normalize();

		FVector Movement = (ForwardDir * ForwardValue + RightDir * RightValue).GetSafeNormal() * MoveSpeed * DeltaTime;
		Pawn->AddActorWorldOffset(Movement, true);
	}

	// 2. Mouse Look (Yaw rotates Pawn, Pitch rotates Camera component)
	float MouseX = 0.0f;
	float MouseY = 0.0f;
	PC->GetInputMouseDelta(MouseX, MouseY);

	if (MouseX != 0.0f)
	{
		FRotator NewRotation = FRotator(0.0f, MouseX * LookSpeed * DeltaTime, 0.0f);
		Pawn->AddActorLocalRotation(NewRotation);
	}

	if (MouseY != 0.0f)
	{
		UCameraComponent* Camera = Pawn->FindComponentByClass<UCameraComponent>();
		if (Camera)
		{
			FRotator CamRot = Camera->GetRelativeRotation();
			// Look up/down (Pitch)
			CamRot.Pitch = FMath::Clamp(CamRot.Pitch - MouseY * LookSpeed * DeltaTime, -80.0f, 80.0f);
			Camera->SetRelativeRotation(CamRot);
		}
	}
}
