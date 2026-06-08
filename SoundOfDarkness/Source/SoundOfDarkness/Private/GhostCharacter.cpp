#include "GhostCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	const TCHAR* DefaultGhostCatchSoundPath = TEXT("/Game/Sound/ghost_catch_oga_cc0.ghost_catch_oga_cc0");
}

AGhostCharacter::AGhostCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsStunned = false;

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultCatchSound(DefaultGhostCatchSoundPath);
	if (DefaultCatchSound.Succeeded())
	{
		CatchSound = DefaultCatchSound.Object;
	}
}

void AGhostCharacter::BeginPlay()
{
	Super::BeginPlay();
	Tags.AddUnique(FName("Ghost"));
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
	}

	if (!CatchSound)
	{
		CatchSound = LoadObject<USoundBase>(nullptr, DefaultGhostCatchSoundPath);
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
		if (!CatchSound)
		{
			CatchSound = LoadObject<USoundBase>(nullptr, DefaultGhostCatchSoundPath);
		}

		if (CatchSound)
		{
			UAudioComponent* CatchAudio = UGameplayStatics::SpawnSound2D(this, CatchSound, CatchSoundVolume, 1.0f, 0.0f, nullptr, true);
			UE_LOG(LogTemp, Warning, TEXT("Spawned ghost catch sound: %s"), CatchAudio ? TEXT("success") : TEXT("failed"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Ghost catch sound is missing. Expected asset: %s"), DefaultGhostCatchSoundPath);
		}
		OnGhostCaughtPlayer.Broadcast(PlayerPawn);
	}
}
