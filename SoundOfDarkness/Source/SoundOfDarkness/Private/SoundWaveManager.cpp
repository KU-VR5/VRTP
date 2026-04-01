#include "SoundWaveManager.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/World.h"

void USoundWaveManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Attempt to load the MPC from a well-known path, we will create this asset later in the engine
    EcholocationMPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/XRFramework/Materials/MPC_Echolocation.MPC_Echolocation"));
    
    for (int i = 0; i < MAX_WAVES; ++i)
    {
        ActiveWaves[i].bIsActive = false;
    }
}

void USoundWaveManager::Deinitialize()
{
    Super::Deinitialize();
}

void USoundWaveManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    bool bNeedsUpdate = false;
    for (int i = 0; i < MAX_WAVES; ++i)
    {
        if (ActiveWaves[i].bIsActive)
        {
            // Expand ripple radius over time
            ActiveWaves[i].CurrentRadius += ActiveWaves[i].ExpansionSpeed * DeltaTime;
            
            // Fade out intensity as it expands based on current vs max radius
            float LifeRatio = FMath::Clamp(ActiveWaves[i].CurrentRadius / ActiveWaves[i].MaxRadius, 0.0f, 1.0f);
            ActiveWaves[i].Intensity = FMath::Lerp(1.0f, 0.0f, LifeRatio);

            // Deactivate completely when fully expanded
            if (LifeRatio >= 1.0f)
            {
                ActiveWaves[i].bIsActive = false;
                ActiveWaves[i].CurrentRadius = 0.0f;
                ActiveWaves[i].Intensity = 0.0f;
            }
            bNeedsUpdate = true;
        }
    }

    if (bNeedsUpdate)
    {
        UpdateMaterialParameters();
    }
}

TStatId USoundWaveManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USoundWaveManager, STATGROUP_Tickables);
}

void USoundWaveManager::SpawnSoundWave(FVector Location, float Radius, float Speed, float InitialIntensity, int32 InPriority)
{
    // Find oldest or inactive wave
    for (int i = 0; i < MAX_WAVES; ++i)
    {
        if (!ActiveWaves[i].bIsActive)
        {
            ActiveWaves[i].Origin = Location;
            ActiveWaves[i].CurrentRadius = 0.0f;
            ActiveWaves[i].MaxRadius = Radius;
            ActiveWaves[i].ExpansionSpeed = Speed;
            ActiveWaves[i].Intensity = InitialIntensity;
            ActiveWaves[i].Priority = InPriority;
            ActiveWaves[i].bIsActive = true;
            return;
        }
    }

    // fallback: simple override of the 0th index if pool is full
    ActiveWaves[0].Origin = Location;
    ActiveWaves[0].CurrentRadius = 0.0f;
    ActiveWaves[0].MaxRadius = Radius;
    ActiveWaves[0].ExpansionSpeed = Speed;
    ActiveWaves[0].Intensity = InitialIntensity;
    ActiveWaves[0].Priority = InPriority;
    ActiveWaves[0].bIsActive = true;
}

void USoundWaveManager::UpdateMaterialParameters()
{
    if (!EcholocationMPC || !GetWorld()) return;

    // Send array data to shader vectors, e.g. Wave0 (Origin, radius), etc.
    // For a parameter collection, we can map X,Y,Z of a vector to Origin, and W to Radius/Intensity encoded
    for (int i = 0; i < MAX_WAVES; ++i)
    {
        FName ParamNameLocation = FName(*FString::Printf(TEXT("WaveLocation_%d"), i));
        FName ParamNameData = FName(*FString::Printf(TEXT("WaveData_%d"), i)); // R = CurrentRadius, G = MaxRadius, B = Intensity

        FVector Location = ActiveWaves[i].bIsActive ? ActiveWaves[i].Origin : FVector::ZeroVector;
        FVector Data = ActiveWaves[i].bIsActive 
            ? FVector(ActiveWaves[i].CurrentRadius, ActiveWaves[i].MaxRadius, ActiveWaves[i].Intensity) 
            : FVector::ZeroVector;

        UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), EcholocationMPC, ParamNameLocation, FLinearColor(Location.X, Location.Y, Location.Z, 1.0f));
        UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), EcholocationMPC, ParamNameData, FLinearColor(Data.X, Data.Y, Data.Z, 1.0f));
    }
}

bool USoundWaveManager::GetHighestPrioritySound(FVector& OutLocation, int32& OutPriority) const
{
    bool bFound = false;
    int32 HighestPriority = -1;

    for (int i = 0; i < MAX_WAVES; ++i)
    {
        if (ActiveWaves[i].bIsActive && ActiveWaves[i].Priority > HighestPriority)
        {
            HighestPriority = ActiveWaves[i].Priority;
            OutLocation = ActiveWaves[i].Origin;
            bFound = true;
        }
    }

    if (bFound)
    {
        OutPriority = HighestPriority;
    }
    
    return bFound;
}
