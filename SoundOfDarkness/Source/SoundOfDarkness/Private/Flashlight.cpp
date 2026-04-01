#include "Flashlight.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AFlashlight::AFlashlight()
{
	PrimaryActorTick.bCanEverTick = true;

	FlashlightBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightBody"));
	RootComponent = FlashlightBody;

	FlashlightLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightLight"));
	FlashlightLight->SetupAttachment(RootComponent);
	FlashlightLight->SetIntensity(5000.f);
	FlashlightLight->SetOuterConeAngle(StunAngleRadius);
	FlashlightLight->SetVisibility(bIsOn);
}

void AFlashlight::BeginPlay()
{
	Super::BeginPlay();
}

void AFlashlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOn)
	{
		CheckForGhostInLight();
	}
}

void AFlashlight::ToggleLight()
{
	bIsOn = !bIsOn;
	FlashlightLight->SetVisibility(bIsOn);
	UE_LOG(LogTemp, Log, TEXT("Flashlight Toggled: %s"), bIsOn ? TEXT("ON") : TEXT("OFF"));
}

void AFlashlight::CheckForGhostInLight()
{
	if (!GetWorld()) return;

	FVector Start = FlashlightLight->GetComponentLocation();
	FVector ForwardVector = FlashlightLight->GetForwardVector();
	FVector End = Start + (ForwardVector * StunDistance);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());

	FHitResult HitResult;

	// Use a simple line trace first to find targets
	bool bHit = UKismetSystemLibrary::LineTraceSingle(
		this, Start, End,
		UEngineTypes::ConvertToTraceType(ECC_WorldDynamic),
		false, ActorsToIgnore,
		EDrawDebugTrace::None, HitResult, true
	);

	if (bHit && HitResult.GetActor())
	{
		// We found something! Check if it's the Ghost.
		// (Assume Ghost will have a Tag "Ghost" for now)
		if (HitResult.GetActor()->ActorHasTag(FName("Ghost")))
		{
			UE_LOG(LogTemp, Warning, TEXT("GHOST STUNNED BY FLASHLIGHT!"));
			
			// Here we would call a function on the Ghost AI to stop it.
			// e.g., HitResult.GetActor()->SetActorTickEnabled(false); or a custom Interface.
		}
	}
}
