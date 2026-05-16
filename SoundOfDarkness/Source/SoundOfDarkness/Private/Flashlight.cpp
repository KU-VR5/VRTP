#include "Flashlight.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GhostCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

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
	RemainingUses = MaxUses;
	FlashlightLight->SetVisibility(bIsOn);
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
	if (bIsOn)
	{
		bIsOn = false;
		FlashlightLight->SetVisibility(false);
		return;
	}

	if (RemainingUses <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Flashlight has no uses remaining."));
		return;
	}

	--RemainingUses;
	bIsOn = true;
	FlashlightLight->SetVisibility(bIsOn);
	GetWorldTimerManager().SetTimerForNextTick(this, &AFlashlight::CheckForGhostInLight);
	FTimerHandle OffTimerHandle;
	GetWorldTimerManager().SetTimer(OffTimerHandle, this, &AFlashlight::TurnOffAfterUse, UseDuration, false);
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

	TArray<FHitResult> HitResults;

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		this, Start, End,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_WorldDynamic),
		false, ActorsToIgnore,
		EDrawDebugTrace::None, HitResults, true
	);

	if (!bHit) return;

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) continue;

		if (AGhostCharacter* Ghost = Cast<AGhostCharacter>(HitActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("GHOST STUNNED BY FLASHLIGHT!"));
			Ghost->StunGhost(StunDuration);
			return;
		}

		if (HitActor->ActorHasTag(FName("Ghost")))
		{
			if (AGhostCharacter* Ghost = Cast<AGhostCharacter>(HitActor))
			{
				Ghost->StunGhost(StunDuration);
				return;
			}
		}
	}
}

void AFlashlight::TurnOffAfterUse()
{
	bIsOn = false;
	FlashlightLight->SetVisibility(false);
}
