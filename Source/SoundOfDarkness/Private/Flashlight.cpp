#include "Flashlight.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GhostCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"

AFlashlight::AFlashlight()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
	DummyRoot->SetMobility(EComponentMobility::Movable);
	RootComponent = DummyRoot;

	FlashlightBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightBody"));
	FlashlightBody->SetupAttachment(RootComponent);
	FlashlightBody->SetMobility(EComponentMobility::Movable);

	FlashlightLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightLight"));
	FlashlightLight->SetupAttachment(RootComponent);
	FlashlightLight->SetMobility(EComponentMobility::Movable);
	FlashlightLight->SetIntensity(300000.f);
	FlashlightLight->SetOuterConeAngle(StunAngleRadius);
	FlashlightLight->SetVisibility(bIsOn);
}

void AFlashlight::BeginPlay()
{
	Super::BeginPlay();
	
	if (FlashlightBody)
	{
		FlashlightBody->SetMobility(EComponentMobility::Movable);
	}
	if (FlashlightLight)
	{
		FlashlightLight->SetMobility(EComponentMobility::Movable);
	}

	RemainingUses = MaxUses;
	FlashlightLight->SetVisibility(bIsOn);
}

void AFlashlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Dynamically align rotation with player's real-time view to guarantee it points where the player is looking
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
		if (PC)
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

			if (!GetAttachParentActor())
			{
				SetActorLocationAndRotation(ViewLocation + ViewRotation.Vector() * 50.f, ViewRotation);
			}
			else
			{
				USceneComponent* AttachParent = GetRootComponent() ? GetRootComponent()->GetAttachParent() : nullptr;
				if (AttachParent && !AttachParent->IsA<UCameraComponent>())
				{
					// Convert view rotation to relative space of the parent
					FRotator RelativeRot = (ViewRotation - OwnerPawn->GetActorRotation());
					SetActorRelativeRotation(RelativeRot);
				}
			}
		}
	}

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

		// Hide debug point light if it exists
		if (UActorComponent* DebugLightComp = GetComponentByClass(UPointLightComponent::StaticClass()))
		{
			if (USceneComponent* DebugLightScene = Cast<USceneComponent>(DebugLightComp))
			{
				DebugLightScene->SetVisibility(false);
			}
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Flashlight Light set to INVISIBLE"));
		}
		return;
	}

	bIsOn = true;
	
	// Force the spotlight to align forward with the actor (in case blueprint rotated it)
	FlashlightLight->SetRelativeLocation(FVector::ZeroVector);
	FlashlightLight->SetRelativeRotation(FRotator::ZeroRotator);

	// Force spotlight to ignore parent scale
	FlashlightLight->SetAbsolute(false, false, true);

	// Force safe values to prevent blueprint overrides from breaking the light visibility
	FlashlightLight->bUseInverseSquaredFalloff = false; // Disable falloff to make it super bright
	FlashlightLight->SetIntensity(300000.f); // Highly visible uniform intensity
	FlashlightLight->SetCastShadows(false); // Prevent the flashlight body or player mesh from blocking the light
	FlashlightLight->SetAttenuationRadius(15000.f); // Increase light range to 150 meters
	FlashlightLight->SetInnerConeAngle(20.f); // Force 20 degrees inner cone
	FlashlightLight->SetOuterConeAngle(45.f); // Force 45 degrees outer cone
	FlashlightLight->SetLightColor(FLinearColor::White); // Force white light color
	FlashlightLight->SetHiddenInGame(false); // Guarantee it is not hidden in game
	FlashlightLight->SetLightFunctionMaterial(nullptr); // Clear any broken light function material
	FlashlightLight->SetIESTexture(nullptr); // Clear any broken IES texture profiles
	FlashlightLight->bAffectsWorld = true; // Ensure it affects the world
	FlashlightLight->SetVisibility(bIsOn);

	if (FlashlightBody)
	{
		FlashlightBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlashlightBody->SetCollisionResponseToAllChannels(ECR_Ignore);
		FlashlightBody->SetHiddenInGame(false);
	}

	// DIAGNOSTIC: Add a 360-degree point light that shines everywhere to guarantee illumination
	UActorComponent* DebugLightComp = GetComponentByClass(UPointLightComponent::StaticClass());
	USceneComponent* DebugLightScene = Cast<USceneComponent>(DebugLightComp);
	if (!DebugLightScene)
	{
		// Spawn dynamically
		if (UPointLightComponent* PointLight = NewObject<UPointLightComponent>(this, TEXT("DebugPointLight")))
		{
			PointLight->SetMobility(EComponentMobility::Movable);
			PointLight->SetAbsolute(false, false, true); // Ignore parent scale
			PointLight->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			PointLight->RegisterComponent();
			DebugLightScene = PointLight;
		}
	}
	if (DebugLightScene)
	{
		if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(DebugLightScene))
		{
			PointLight->bUseInverseSquaredFalloff = false; // Disable falloff
			PointLight->SetIntensity(50000.f); // Safe constant intensity
			PointLight->SetCastShadows(false);
			PointLight->SetLightColor(FLinearColor::White);
			PointLight->SetHiddenInGame(false);
			PointLight->bAffectsWorld = true;
		}
		DebugLightScene->SetVisibility(true);
	}
	
	GetWorldTimerManager().SetTimerForNextTick(this, &AFlashlight::CheckForGhostInLight);
	
	UE_LOG(LogTemp, Log, TEXT("Flashlight Toggled: %s"), bIsOn ? TEXT("ON") : TEXT("OFF"));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Flashlight VISIBLE! (Permanent Toggle Mode)"));
	}
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
