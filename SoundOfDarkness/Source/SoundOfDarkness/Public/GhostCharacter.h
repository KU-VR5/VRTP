#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GhostCharacter.generated.h"

class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGhostCaughtPlayer, APawn*, PlayerPawn);

UCLASS()
class SOUNDOFDARKNESS_API AGhostCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGhostCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// Called when the ghost gets hit by the flashlight
	UFUNCTION(BlueprintCallable, Category = "AI|Ghost")
	void StunGhost(float StunDuration);

	UFUNCTION(BlueprintCallable, Category = "AI|Ghost")
	bool IsStunned() const { return bIsStunned; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Ghost")
	bool bIsStunned = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ghost")
	float CatchDistance = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ghost")
	bool bCanCatchPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ghost|Audio")
	USoundBase* CatchSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ghost|Audio", meta = (ClampMin = "0.0"))
	float CatchSoundVolume = 1.0f;

	UPROPERTY(BlueprintAssignable, Category = "AI|Ghost")
	FOnGhostCaughtPlayer OnGhostCaughtPlayer;

private:
	FTimerHandle StunTimerHandle;
	float OriginalMaxWalkSpeed = 0.0f;

	void EndStun();
	void CheckCatchPlayer();
};
