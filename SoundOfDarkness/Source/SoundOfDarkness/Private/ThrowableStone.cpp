#include "ThrowableStone.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SoundWaveManager.h"

AThrowableStone::AThrowableStone()
{
	PrimaryActorTick.bCanEverTick = false;

	StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneMesh"));
	RootComponent = StoneMesh;

	// Enable physics and simulation by default
	StoneMesh->SetSimulatePhysics(true);
	StoneMesh->SetNotifyRigidBodyCollision(true); // Required to detect hits

	// Bind hit function dynamically
	StoneMesh->OnComponentHit.AddDynamic(this, &AThrowableStone::OnStoneHit);
}

void AThrowableStone::BeginPlay()
{
	Super::BeginPlay();
}

void AThrowableStone::OnStoneHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) return;
	if (!GetWorld()) return;
	
	// Check velocity to prevent tiny slides triggering massive sound waves repeatedly
	float CurrentSpeed = GetVelocity().Size();
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentSpeed >= MinimumImpactVelocity && CurrentTime - LastImpactTime >= ImpactCooldown)
	{
		LastImpactTime = CurrentTime;

		// Obtain the World Subsystem
		USoundWaveManager* SoundManager = GetWorld()->GetSubsystem<USoundWaveManager>();
		if (SoundManager)
		{
			// Spawn a sound wave at the hit location (Priority 50 = Stone)
			SoundManager->SpawnSoundWave(Hit.ImpactPoint, ImpactWaveRadius, ImpactWaveSpeed, ImpactWaveIntensity, 50);
			
			// Optional: Alert AI here by emitting an AI noise event
			// UGameplayStatics::PlaySoundAtLocation(...)
		}
	}
}
