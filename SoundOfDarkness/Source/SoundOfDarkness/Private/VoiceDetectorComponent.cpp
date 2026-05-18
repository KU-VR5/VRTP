#include "VoiceDetectorComponent.h"
#include "AudioCaptureComponent.h"
#include "SoundWaveManager.h"
#include "Kismet/GameplayStatics.h"

UVoiceDetectorComponent::UVoiceDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
    LoudnessThreshold = 0.3f; // 0 to 1 value
    EcholocationRadiusModifier = 2500.0f;
    WaveCooldown = 1.0f;
    VoiceWaveSpeed = 1800.0f;
    TimeSinceLastWave = 0.0f;
}

void UVoiceDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

    // Create the AudioCaptureComponent dynamically since it's an engine plugin
    AudioCaptureComp = NewObject<UAudioCaptureComponent>(this, UAudioCaptureComponent::StaticClass());
    if (AudioCaptureComp)
    {
        AudioCaptureComp->EnvelopeFollowerAttackTime = 10;
        AudioCaptureComp->EnvelopeFollowerReleaseTime = 80;
        AudioCaptureComp->OnAudioEnvelopeValue.AddDynamic(this, &UVoiceDetectorComponent::HandleAudioEnvelope);
        AudioCaptureComp->RegisterComponent();
        AudioCaptureComp->Start();
    }
}

void UVoiceDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastWave += DeltaTime;
}

void UVoiceDetectorComponent::HandleAudioEnvelope(const float EnvelopeValue)
{
    if (!GetWorld() || TimeSinceLastWave < WaveCooldown) return;
    if (EnvelopeValue < LoudnessThreshold) return;

    TimeSinceLastWave = 0.0f;
    OnVoiceLoudnessExceeded.Broadcast(EnvelopeValue);

    if (USoundWaveManager* SoundManager = GetWorld()->GetSubsystem<USoundWaveManager>())
    {
        const FVector Origin = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
        const float Radius = FMath::Clamp(EnvelopeValue, 0.0f, 1.0f) * EcholocationRadiusModifier;
        SoundManager->SpawnSoundWave(Origin, Radius, VoiceWaveSpeed, EnvelopeValue, 100);
    }
}
