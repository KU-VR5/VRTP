#include "MazeRandomSpawner.h"
#include "Engine/World.h"

AMazeRandomSpawner::AMazeRandomSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMazeRandomSpawner::BeginPlay()
{
	Super::BeginPlay();

	RandomStream.Initialize(bUseRandomSeed ? FMath::Rand() : Seed);

	if (bSpawnOnBeginPlay)
	{
		SpawnRandomActors();
	}
}

void AMazeRandomSpawner::SpawnRandomActors()
{
	if (!GetWorld() || ActorClasses.IsEmpty() || SpawnPoints.IsEmpty()) return;

	TArray<AActor*> AvailableSpawnPoints = SpawnPoints;
	if (bShuffleSpawnPoints)
	{
		for (int32 Index = AvailableSpawnPoints.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = RandomStream.RandRange(0, Index);
			AvailableSpawnPoints.Swap(Index, SwapIndex);
		}
	}

	const int32 DesiredCount = SpawnCount > 0 ? SpawnCount : AvailableSpawnPoints.Num();
	const int32 SafeSpawnCount = FMath::Min(DesiredCount, AvailableSpawnPoints.Num());

	for (int32 Index = 0; Index < SafeSpawnCount; ++Index)
	{
		AActor* SpawnPoint = AvailableSpawnPoints[Index];
		if (!SpawnPoint) continue;

		const int32 ClassIndex = RandomStream.RandRange(0, ActorClasses.Num() - 1);
		TSubclassOf<AActor> ActorClass = ActorClasses[ClassIndex];
		if (!ActorClass) continue;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AActor>(ActorClass, SpawnPoint->GetActorTransform(), SpawnParams);
	}
}
