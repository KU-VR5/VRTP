#include "MazeNoiseComponent.h"
#include "GameFramework/Actor.h"
#include "SoundWaveManager.h"

UMazeNoiseComponent::UMazeNoiseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMazeNoiseComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMazeNoiseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEmitMovementWaves || !GetOwner()) return;

	TimeSinceLastFootstep += DeltaTime;
	const float Speed2D = GetOwner()->GetVelocity().Size2D();
	const bool bIsMoving = Speed2D >= MinMoveSpeed;

	if (bIsMoving && !bWasMoving)
	{
		TimeSinceLastFootstep = 0.0f;
		EmitNoiseWave(FootstepWaveRadius, FootstepWaveSpeed, FootstepWaveIntensity, FootstepPriority);
	}
	else if (bIsMoving && TimeSinceLastFootstep >= FootstepInterval)
	{
		TimeSinceLastFootstep = 0.0f;
		EmitNoiseWave(FootstepWaveRadius, FootstepWaveSpeed, FootstepWaveIntensity, FootstepPriority);
	}
	else if (!bIsMoving)
	{
		TimeSinceLastFootstep = FootstepInterval;
	}

	bWasMoving = bIsMoving;
}

void UMazeNoiseComponent::EmitNoiseWave(float Radius, float Speed, float Intensity, int32 Priority)
{
	if (!GetWorld() || !GetOwner()) return;

	if (USoundWaveManager* SoundManager = GetWorld()->GetSubsystem<USoundWaveManager>())
	{
		SoundManager->SpawnSoundWave(GetOwner()->GetActorLocation(), Radius, Speed, Intensity, Priority);
	}
}
