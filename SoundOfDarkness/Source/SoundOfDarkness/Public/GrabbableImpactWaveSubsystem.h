#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GrabbableImpactWaveSubsystem.generated.h"

class UPrimitiveComponent;

UCLASS()
class SOUNDOFDARKNESS_API UGrabbableImpactWaveSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

private:
	UPROPERTY()
	TArray<TObjectPtr<UPrimitiveComponent>> BoundComponents;

	TMap<TWeakObjectPtr<UPrimitiveComponent>, float> LastImpactTimes;

	float TimeSinceLastScan = 0.0f;
	float ScanInterval = 0.5f;
	float MinimumImpactSpeed = 50.0f;
	float ImpactCooldown = 0.35f;
	float ImpactWaveRadius = 2000.0f;
	float ImpactWaveSpeed = 1500.0f;
	float ImpactWaveIntensity = 1.0f;

	void ScanAndBindGrabbables();
	bool IsGrabbableActor(const AActor* Actor) const;
	bool IsMazeImpactTarget(const AActor* Actor, const UPrimitiveComponent* Component) const;
	void ConfigureImpactComponent(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleGrabbableHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
