#include "VoiceDetectorComponent.h"
#include "AudioCaptureComponent.h"
#include "SoundWaveManager.h"
#include "Kismet/GameplayStatics.h"

UVoiceDetectorComponent::UVoiceDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
    LoudnessThreshold = 0.3f; // 0 to 1 value
    EcholocationRadiusModifier = 2500.0f;
    TimeSinceLastWave = 0.0f;
}

void UVoiceDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

    // Create the AudioCaptureComponent dynamically since it's an engine plugin
    AudioCaptureComp = NewObject<UAudioCaptureComponent>(this, UAudioCaptureComponent::StaticClass());
    if (AudioCaptureComp)
    {
        AudioCaptureComp->RegisterComponent();
        AudioCaptureComp->Start();
    }
}

void UVoiceDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (AudioCaptureComp)
    {
        // For UE5 AudioCapture plugin, we typically bind to OnAudioEnvelopeValue to get mic volume.
        // We will expose this functionality so Blueprint can configure the capture source.
    }
}
