#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PCDemoPlayerController.generated.h"

class AThrowableStone;

UCLASS()
class SOUNDOFDARKNESS_API APCDemoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APCDemoPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	// Inject keyboard and mouse input settings at runtime
	void SetupDynamicInputSettings();

	// Input Axis Callbacks
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// Input Action Callbacks
	void InteractOrThrow();

	// Interaction actions
	void TryPickupStone();
	void ThrowStone();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC Demo Settings")
	float MoveSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC Demo Settings")
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC Demo Settings")
	float ThrowForce = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC Demo Settings")
	float GrabDistance = 400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PC Demo Settings")
	bool bIsHoldingStone = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PC Demo Settings")
	AThrowableStone* HeldStone = nullptr;
};
