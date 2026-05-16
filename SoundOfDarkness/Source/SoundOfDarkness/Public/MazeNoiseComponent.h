#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MazeNoiseComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUNDOFDARKNESS_API UMazeNoiseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMazeNoiseComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Maze Noise")
	void EmitNoiseWave(float Radius, float Speed, float Intensity, int32 Priority);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Noise|Footsteps")
	bool bEmitMovementWaves = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Noise|Footsteps")
	float MinMoveSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Noise|Footsteps")
	float FootstepInterval = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Noise|Footsteps")
	float FootstepWaveRadius = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Noise|Footsteps")
	float FootstepWaveSpeed = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Noise|Footsteps")
	float FootstepWaveIntensity = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Noise|Footsteps")
	int32 FootstepPriority = 25;

private:
	float TimeSinceLastFootstep = 0.0f;
};
