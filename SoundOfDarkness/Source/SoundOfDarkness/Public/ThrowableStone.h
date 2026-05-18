#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowableStone.generated.h"

class UStaticMeshComponent;

UCLASS()
class SOUNDOFDARKNESS_API AThrowableStone : public AActor
{
	GENERATED_BODY()
	
public:	
	AThrowableStone();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StoneMesh;

	// The loudness or intensity of the sound wave when the stone hits something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Wave Settings")
	float ImpactWaveIntensity = 1.0f;

	// Maximum radius the ripple will expand to when thrown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Wave Settings")
	float ImpactWaveRadius = 2000.0f;

	// How fast the ripple expands
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Wave Settings")
	float ImpactWaveSpeed = 1500.0f;

	// Minimum velocity required to trigger a sound wave upon hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Wave Settings")
	float MinimumImpactVelocity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Wave Settings")
	float ImpactCooldown = 0.8f;

	UFUNCTION()
	void OnStoneHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	float LastImpactTime = -1000.0f;
};
