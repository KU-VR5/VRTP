#include "FlashlightControllerComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include "Components/ChildActorComponent.h"
#include "Flashlight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"


UFlashlightControllerComponent::UFlashlightControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UFlashlightControllerComponent::BeginPlay()
{
	Super::BeginPlay();
	BindInput();
}

void UFlashlightControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Fallback in case Pawn was possessed and InputComponent was initialized after BeginPlay
	if (!bInputBound)
	{
		BindInput();
	}
}

void UFlashlightControllerComponent::BindInput()
{
	if (bInputBound) return;

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->InputComponent)
	{
		Pawn->InputComponent->BindKey(EKeys::C, IE_Pressed, this, &UFlashlightControllerComponent::OnCKeyPressed);
		bInputBound = true;
		
		// Disable tick since we no longer need to check for InputComponent
		SetComponentTickEnabled(false);
		UE_LOG(LogTemp, Log, TEXT("FlashlightControllerComponent: Successfully bound 'C' key to toggle flashlight."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("FlashlightControllerComponent: Successfully bound 'C' key."));
		}
	}
}

void UFlashlightControllerComponent::OnCKeyPressed()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("C Key Pressed! Searching for Flashlight..."));
	}

	// Get player eye level view point (works for both VR and PC)
	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// Target location: 50 units in front of player eyes, pointing forward
	FVector TargetWorldLocation = ViewLocation + (ViewRotation.Vector() * 50.f);
	FRotator TargetWorldRotation = ViewRotation;

	UCameraComponent* Camera = OwnerPawn->FindComponentByClass<UCameraComponent>();

	auto PositionFlashlight = [&](AActor* FlashlightActor)
	{
		if (FlashlightActor)
		{
			// 1. Disable all collision on the flashlight actor and its components to prevent physics/character pushing bugs
			FlashlightActor->SetActorEnableCollision(false);
			
			TArray<UPrimitiveComponent*> PrimitiveComps;
			FlashlightActor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
			for (UPrimitiveComponent* Primitive : PrimitiveComps)
			{
				if (Primitive)
				{
					Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					Primitive->SetCollisionResponseToAllChannels(ECR_Ignore);
				}
			}

			// 2. Force Actor Visibility and Scale
			FlashlightActor->SetActorHiddenInGame(false);
			FlashlightActor->SetActorScale3D(FVector(1.f, 1.f, 1.f));

			// 3. Attach using clean snapping rules (if not already attached to our pawn)
			USceneComponent* AttachParent = Camera ? Cast<USceneComponent>(Camera) : OwnerPawn->GetRootComponent();
			if (AttachParent)
			{
				if (FlashlightActor->GetAttachParentActor() != OwnerPawn)
				{
					FlashlightActor->AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				}
				
				// Set clean relative offset depending on whether we attach to Camera or Pawn Root Component
				if (Camera)
				{
					FlashlightActor->SetActorRelativeLocation(FVector(50.f, 0.f, -10.f));
					FlashlightActor->SetActorRelativeRotation(FRotator::ZeroRotator);
				}
				else
				{
					// If attached to root, place it at approximate eye level (60 units up) and 50 units forward
					FlashlightActor->SetActorRelativeLocation(FVector(50.f, 0.f, 60.f));
					FlashlightActor->SetActorRelativeRotation(FRotator::ZeroRotator);
				}
			}

			if (GEngine)
			{
				FString CoordMsg = FString::Printf(TEXT("Flashlight Placed Relative to Parent (Scale: 1.0)"));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, CoordMsg);
			}
		}
	};

	// 1. Check for attached Flashlight actors
	TArray<AActor*> AttachedActors;
	OwnerPawn->GetAttachedActors(AttachedActors);
	for (AActor* Actor : AttachedActors)
	{
		if (AFlashlight* Flashlight = Cast<AFlashlight>(Actor))
		{
			PositionFlashlight(Flashlight);
			Flashlight->ToggleLight();
			UE_LOG(LogTemp, Log, TEXT("FlashlightControllerComponent: Toggled attached Flashlight actor."));
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Flashlight (Attached) Toggled!"));
			}
			return;
		}
	}

	// 2. Check for ChildActorComponent containing a Flashlight
	TArray<UChildActorComponent*> ChildActorComponents;
	OwnerPawn->GetComponents<UChildActorComponent>(ChildActorComponents);
	for (UChildActorComponent* Comp : ChildActorComponents)
	{
		if (Comp && Comp->GetChildActorClass() && Comp->GetChildActorClass()->IsChildOf(AFlashlight::StaticClass()))
		{
			if (AFlashlight* Flashlight = Cast<AFlashlight>(Comp->GetChildActor()))
			{
				// Snapping component to camera/face if we are testing on PC
				USceneComponent* AttachParent = Camera ? Cast<USceneComponent>(Camera) : OwnerPawn->GetRootComponent();
				if (AttachParent)
				{
					Comp->AttachToComponent(AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					Comp->SetRelativeLocation(FVector(50.f, 0.f, -10.f));
					Comp->SetRelativeRotation(FRotator::ZeroRotator);
				}

				Flashlight->ToggleLight();
				UE_LOG(LogTemp, Log, TEXT("FlashlightControllerComponent: Toggled Flashlight in ChildActorComponent."));
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Flashlight (ChildActorComponent) Toggled!"));
				}
				return;
			}
		}
	}

	// 3. Fallback: search for any Flashlight actor owned by this Pawn in the world
	for (TActorIterator<AFlashlight> It(GetWorld()); It; ++It)
	{
		if (It->GetOwner() == OwnerPawn)
		{
			PositionFlashlight(*It);
			It->ToggleLight();
			UE_LOG(LogTemp, Log, TEXT("FlashlightControllerComponent: Toggled owned Flashlight actor in the world."));
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Flashlight (Owned in World) Toggled!"));
			}
			return;
		}
	}

	// 4. Auto-Spawn Fallback: Spawn and attach a temporary flashlight for testing
	if (GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerPawn;
		SpawnParams.Instigator = OwnerPawn;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		USceneComponent* AttachParent = Camera ? Cast<USceneComponent>(Camera) : OwnerPawn->GetRootComponent();
		
		AFlashlight* SpawnedFlashlight = GetWorld()->SpawnActor<AFlashlight>(AFlashlight::StaticClass(), TargetWorldLocation, TargetWorldRotation, SpawnParams);
		if (SpawnedFlashlight)
		{
			PositionFlashlight(SpawnedFlashlight);
			SpawnedFlashlight->ToggleLight();
			
			UE_LOG(LogTemp, Warning, TEXT("FlashlightControllerComponent: Flashlight not found. Spawned and attached a temporary one."));
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Flashlight Auto-Spawned and Attached!"));
			}
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("FlashlightControllerComponent: 'C' pressed, and failed to spawn fallback Flashlight."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("C pressed, and failed to spawn Flashlight!"));
	}
}
