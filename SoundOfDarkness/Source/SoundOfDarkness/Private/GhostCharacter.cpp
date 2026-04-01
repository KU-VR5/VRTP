#include "GhostCharacter.h"
#include "TimerManager.h"

AGhostCharacter::AGhostCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsStunned = false;
}

void AGhostCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AGhostCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGhostCharacter::StunGhost(float StunDuration)
{
	if (bIsStunned) return;

	bIsStunned = true;
	
	// Disable movement logic here if needed, or simply pass to Blackboard
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &AGhostCharacter::EndStun, StunDuration, false);
}

void AGhostCharacter::EndStun()
{
	bIsStunned = false;
	GetWorldTimerManager().ClearTimer(StunTimerHandle);
}
