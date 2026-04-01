#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GhostAIController.generated.h"

UCLASS()
class SOUNDOFDARKNESS_API AGhostAIController : public AAIController
{
	GENERATED_BODY()

public:
    AGhostAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // The name of the Blackboard Key used to store the destination
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName TargetLocationKeyName = "TargetLocation";

    // How often to check for new sound waves
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float PollingInterval = 0.5f;

private:
    float TimeSinceLastPoll;

    void PollSoundWaves();
};
