#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeRandomSpawner.generated.h"

UCLASS()
class SOUNDOFDARKNESS_API AMazeRandomSpawner : public AActor
{
	GENERATED_BODY()

public:
	AMazeRandomSpawner();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Spawner")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Spawner")
	bool bUseRandomSeed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Spawner")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Spawner")
	TArray<TSubclassOf<AActor>> ActorClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Spawner")
	TArray<AActor*> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Spawner", meta = (ClampMin = "0"))
	int32 SpawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Spawner")
	bool bShuffleSpawnPoints = true;

	UFUNCTION(BlueprintCallable, Category = "Random Spawner")
	void SpawnRandomActors();

private:
	FRandomStream RandomStream;
};
