#include "MazeObjectiveAutoPlacer.h"
#include "FusePickup.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

AMazeObjectiveAutoPlacer::AMazeObjectiveAutoPlacer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMazeObjectiveAutoPlacer::BeginPlay()
{
	Super::BeginPlay();

	RandomStream.Initialize(bUseRandomSeed ? FMath::Rand() : Seed);
	if (!FusePickupClass)
	{
		FusePickupClass = AFusePickup::StaticClass();
	}

	if (bSpawnOnBeginPlay)
	{
		SpawnFuses();
	}
}

void AMazeObjectiveAutoPlacer::SpawnFuses()
{
	UWorld* World = GetWorld();
	if (!World || !FusePickupClass || FuseCount <= 0) return;

	if (bDestroyExistingAutoPlacedFuses)
	{
		DestroyExistingAutoPlacedFuses();
	}

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	const FVector Origin = GetPlacementOrigin();
	TArray<FVector> SpawnedLocations;

	for (int32 FuseIndex = 0; FuseIndex < FuseCount; ++FuseIndex)
	{
		bool bPlaced = false;
		for (int32 Attempt = 0; Attempt < 80 && !bPlaced; ++Attempt)
		{
			FVector Candidate = Origin;
			if (NavSystem)
			{
				FNavLocation NavLocation;
				if (!NavSystem->GetRandomReachablePointInRadius(Origin, SearchRadius, NavLocation)) continue;
				Candidate = NavLocation.Location;
			}
			else
			{
				const float Angle = RandomStream.FRandRange(0.0f, TWO_PI);
				const float Radius = RandomStream.FRandRange(MinDistanceFromPlayer, SearchRadius);
				Candidate += FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
			}

			if (FVector::DistSquared2D(Candidate, Origin) < FMath::Square(MinDistanceFromPlayer)) continue;
			if (!IsFarEnoughFromExisting(Candidate, SpawnedLocations)) continue;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AFusePickup* Fuse = World->SpawnActor<AFusePickup>(FusePickupClass, Candidate + FVector(0.0f, 0.0f, 40.0f), FRotator::ZeroRotator, SpawnParams);
			if (Fuse)
			{
				Fuse->Tags.AddUnique(FName(TEXT("AutoPlacedFuse")));
				SpawnedLocations.Add(Fuse->GetActorLocation());
				bPlaced = true;
			}
		}
	}
}

FVector AMazeObjectiveAutoPlacer::GetPlacementOrigin() const
{
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return PlayerPawn->GetActorLocation();
	}

	if (AActor* PlayerStart = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass()))
	{
		return PlayerStart->GetActorLocation();
	}

	return GetActorLocation();
}

bool AMazeObjectiveAutoPlacer::IsFarEnoughFromExisting(const FVector& Candidate, const TArray<FVector>& ExistingLocations) const
{
	for (const FVector& ExistingLocation : ExistingLocations)
	{
		if (FVector::DistSquared2D(Candidate, ExistingLocation) < FMath::Square(MinDistanceBetweenFuses))
		{
			return false;
		}
	}

	return true;
}

void AMazeObjectiveAutoPlacer::DestroyExistingAutoPlacedFuses()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AFusePickup> It(World); It; ++It)
	{
		AFusePickup* Fuse = *It;
		if (Fuse && Fuse->ActorHasTag(FName(TEXT("AutoPlacedFuse"))))
		{
			Fuse->Destroy();
		}
	}
}
