#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SoundWaveManager.generated.h"

class UMaterialParameterCollection;

USTRUCT(BlueprintType)
struct FSoundWaveRipple
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Origin = FVector::ZeroVector;

    UPROPERTY()
    float CurrentRadius = 0.0f;

    UPROPERTY()
    float MaxRadius = 1000.0f;

    UPROPERTY()
    float ExpansionSpeed = 800.0f;

    UPROPERTY()
    float Intensity = 1.0f;

    UPROPERTY()
    bool bIsActive = false;

    UPROPERTY()
    int32 Priority = 0;
};

/**
 * Subsystem to manage expanding sound ripples for the echolocation shader.
 */
UCLASS()
class SOUNDOFDARKNESS_API USoundWaveManager : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // UTickableWorldSubsystem interfaces
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

    // Spawn a new sound ripple. Higher priority sounds (e.g. Voice = 100, Stone = 50) override Ghost AI.
    UFUNCTION(BlueprintCallable, Category = "Echolocation")
    void SpawnSoundWave(FVector Location, float Radius, float Speed, float InitialIntensity, int32 Priority = 0);

    // Get the location of the highest priority active sound
    UFUNCTION(BlueprintCallable, Category = "Echolocation")
    bool GetHighestPrioritySound(FVector& OutLocation, int32& OutPriority) const;

protected:
    UPROPERTY()
    UMaterialParameterCollection* EcholocationMPC;

private:
    static const int32 MAX_WAVES = 8;
    FSoundWaveRipple ActiveWaves[MAX_WAVES];

    void UpdateMaterialParameters();
};
