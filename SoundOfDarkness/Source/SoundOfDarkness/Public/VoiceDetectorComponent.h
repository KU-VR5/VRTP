#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceDetectorComponent.generated.h"

class UAudioCaptureComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceLoudnessExceeded, float, Loudness);

/**
 * Component that attaches to the player to detect microphone input level using AudioCapture.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOUNDOFDARKNESS_API UVoiceDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVoiceDetectorComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Minimum envelope value (0.0 to 1.0) to register as a sound wave source
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio detection")
    float LoudnessThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio detection")
    float EcholocationRadiusModifier;

    UPROPERTY(BlueprintAssignable, Category = "Audio detection")
    FOnVoiceLoudnessExceeded OnVoiceLoudnessExceeded;

private:
    UPROPERTY()
    UAudioCaptureComponent* AudioCaptureComp;

    float TimeSinceLastWave;
};
