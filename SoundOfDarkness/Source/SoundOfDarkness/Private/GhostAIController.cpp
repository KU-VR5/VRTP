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
    // Don't poll if we don't have a blackboard initialized yet or the ghost is stunned
    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (!BlackboardComp) return;

    AGhostCharacter* MyGhost = Cast<AGhostCharacter>(GetPawn());
    if (MyGhost && MyGhost->bIsStunned)
    {
        // Don't chase sounds while stunned
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
            // Update the Blackboard with the coordinate so the Behavior Tree moves the ghost there!
            BlackboardComp->SetValueAsVector(TargetLocationKeyName, TargetLoc);
            MoveToLocation(TargetLoc, AcceptanceRadius, true, true, true, false, nullptr, true);
        }
    }
}
