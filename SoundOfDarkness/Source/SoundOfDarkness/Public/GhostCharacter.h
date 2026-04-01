#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GhostCharacter.generated.h"

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Ghost")
	bool bIsStunned = false;

private:
	FTimerHandle StunTimerHandle;

	void EndStun();
};
