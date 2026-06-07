#include "GhostCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "MazeGameFlowManager.h"

AGhostCharacter::AGhostCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsStunned = false;
	CatchDistance = 180.0f; // Increase default catch distance for reliability
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

	float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation());

	// Output distance in real-time when the ghost is close (within 5 meters)
	if (Distance < 500.f && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Red, FString::Printf(TEXT("Ghost Distance to Player: %.1f (Limit: %.1f)"), Distance, CatchDistance));
	}

	if (Distance <= CatchDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player caught by ghost. Distance: %.1f"), Distance);
		bCanCatchPlayer = false;
		OnGhostCaughtPlayer.Broadcast(PlayerPawn);

		// Directly notify the Game Flow Manager to trigger Game Over / Lose Game state
		AMazeGameFlowManager* FlowManager = Cast<AMazeGameFlowManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMazeGameFlowManager::StaticClass()));
		if (!FlowManager)
		{
			// If not found in the world, spawn the FlowManager dynamically
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FlowManager = GetWorld()->SpawnActor<AMazeGameFlowManager>(AMazeGameFlowManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}

		if (FlowManager)
		{
			FlowManager->LoseGame();
		}
	}
}
