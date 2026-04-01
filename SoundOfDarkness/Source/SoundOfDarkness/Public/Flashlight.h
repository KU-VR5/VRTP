#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flashlight.generated.h"

class UStaticMeshComponent;
class USpotLightComponent;

UCLASS()
class SOUNDOFDARKNESS_API AFlashlight : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlashlight();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void ToggleLight();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
    UStaticMeshComponent* FlashlightBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
    USpotLightComponent* FlashlightLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight")
    float StunDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight")
    float StunAngleRadius = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight")
    bool bIsOn = false;

private:
    void CheckForGhostInLight();
};
