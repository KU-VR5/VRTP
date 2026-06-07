#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeObjectiveAutoPlacer.generated.h"

class AFusePickup;

UCLASS()
class SOUNDOFDARKNESS_API AMazeObjectiveAutoPlacer : public AActor
{
	GENERATED_BODY()

public:
	AMazeObjectiveAutoPlacer();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement")
	bool bUseRandomSeed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement")
	int32 Seed = 2026;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement")
	TSubclassOf<AFusePickup> FusePickupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement", meta = (ClampMin = "0"))
	int32 FuseCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement", meta = (ClampMin = "0.0"))
	float SearchRadius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement", meta = (ClampMin = "0.0"))
	float MinDistanceFromPlayer = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement", meta = (ClampMin = "0.0"))
	float MinDistanceBetweenFuses = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective Placement")
	bool bDestroyExistingAutoPlacedFuses = true;

	UFUNCTION(BlueprintCallable, Category = "Objective Placement")
	void SpawnFuses();

private:
	FRandomStream RandomStream;
	FVector GetPlacementOrigin() const;
	bool IsFarEnoughFromExisting(const FVector& Candidate, const TArray<FVector>& ExistingLocations) const;
	void DestroyExistingAutoPlacedFuses();
};
