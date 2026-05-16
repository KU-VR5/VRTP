#include "GhostCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AGhostCharacter::AGhostCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsStunned = false;
}

void AGhostCharacter::BeginPlay()
{
	Super::BeginPlay();
	Tags.AddUnique(FName("Ghost"));
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
	}
}

void AGhostCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckCatchPlayer();
}

void AGhostCharacter::StunGhost(float StunDuration)
{
	if (bIsStunned) return;

	bIsStunned = true;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->MaxWalkSpeed = 0.0f;
	}
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &AGhostCharacter::EndStun, StunDuration, false);
}

void AGhostCharacter::EndStun()
{
	bIsStunned = false;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}
	GetWorldTimerManager().ClearTimer(StunTimerHandle);
}

void AGhostCharacter::CheckCatchPlayer()
{
	if (!bCanCatchPlayer || bIsStunned || !GetWorld()) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	if (FVector::DistSquared(PlayerPawn->GetActorLocation(), GetActorLocation()) <= FMath::Square(CatchDistance))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player caught by ghost."));
		bCanCatchPlayer = false;
	}
}
