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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    bool bDrawDebugRipples = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    bool bDrawEchoOutlines = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float DebugRingHeightOffset = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float DebugRingThickness = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    int32 OutlineRayCount = 144;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    int32 OutlineVerticalSamples = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float OutlineSampleHeight = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float OutlineVerticalSpan = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float OutlineStrokeLength = 55.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float OutlineThickness = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float OutlineLifetime = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    bool bDrawHorizontalSurfaceOutlines = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual", meta = (ClampMin = "1"))
    int32 SurfaceRadialSamples = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float FloorTraceStartOffset = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float FloorTraceDepth = 220.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float CeilingTraceStartOffset = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echolocation|Visual")
    float CeilingTraceHeight = 900.0f;

protected:
    UPROPERTY()
    UMaterialParameterCollection* EcholocationMPC;

private:
    static const int32 MAX_WAVES = 8;
    FSoundWaveRipple ActiveWaves[MAX_WAVES];

    void UpdateMaterialParameters();
    void DrawDebugRipples() const;
    void DrawEchoOutlines() const;
    FColor GetWaveColor(const FSoundWaveRipple& Wave) const;
};
