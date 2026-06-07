#include "GhostAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "SoundWaveManager.h"
#include "GhostCharacter.h"

AGhostAIController::AGhostAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    TimeSinceLastPoll = 0.0f;
}

void AGhostAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AGhostAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    TimeSinceLastPoll += DeltaSeconds;
    if (TimeSinceLastPoll >= PollingInterval)
    {
        PollSoundWaves();
        TimeSinceLastPoll = 0.0f;
    }
}

void AGhostAIController::PollSoundWaves()
{
    // Don't chase sounds while stunned.
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    AGhostCharacter* MyGhost = Cast<AGhostCharacter>(GetPawn());
    if (MyGhost && MyGhost->bIsStunned)
    {
        return;
    }

    USoundWaveManager* SoundManager = GetWorld()->GetSubsystem<USoundWaveManager>();
    if (SoundManager)
    {
        FVector TargetLoc;
        int32 MaxPriority;
        
        // Find the most relevant active sound
        if (SoundManager->GetHighestPrioritySound(TargetLoc, MaxPriority))
        {
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsVector(TargetLocationKeyName, TargetLoc);
            }

            if (BlackboardComp || bMoveWithoutBlackboard)
            {
                MoveToLocation(TargetLoc, AcceptanceRadius, true, true, true, false, nullptr, true);
            }
        }
    }
}
