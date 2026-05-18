#include "SoundWaveManager.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

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
        DrawDebugRipples();
        DrawEchoOutlines();
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

void USoundWaveManager::DrawDebugRipples() const
{
    if (!bDrawDebugRipples || !GetWorld()) return;

    for (int i = 0; i < MAX_WAVES; ++i)
    {
        if (!ActiveWaves[i].bIsActive) continue;

        const FVector Center = ActiveWaves[i].Origin + FVector(0.0f, 0.0f, DebugRingHeightOffset);
        DrawDebugCircle(
            GetWorld(),
            Center,
            ActiveWaves[i].CurrentRadius,
            96,
            GetWaveColor(ActiveWaves[i]),
            false,
            0.05f,
            0,
            DebugRingThickness,
            FVector::ForwardVector,
            FVector::RightVector,
            false
        );
    }
}

void USoundWaveManager::DrawEchoOutlines() const
{
    if (!bDrawEchoOutlines || !GetWorld()) return;

    const int32 SafeRayCount = FMath::Max(OutlineRayCount, 16);
    const int32 SafeVerticalSamples = FMath::Max(OutlineVerticalSamples, 1);
    const int32 SafeSurfaceRadialSamples = FMath::Max(SurfaceRadialSamples, 1);

    for (int WaveIndex = 0; WaveIndex < MAX_WAVES; ++WaveIndex)
    {
        const FSoundWaveRipple& Wave = ActiveWaves[WaveIndex];
        if (!Wave.bIsActive || Wave.CurrentRadius <= 1.0f) continue;

        const float Fade = FMath::Clamp(Wave.Intensity, 0.05f, 1.0f);
        const FColor OutlineColor = GetWaveColor(Wave).WithAlpha(static_cast<uint8>(Fade * 255.0f));

        for (int32 RayIndex = 0; RayIndex < SafeRayCount; ++RayIndex)
        {
            const float Angle = (TWO_PI * static_cast<float>(RayIndex)) / static_cast<float>(SafeRayCount);
            const FVector Direction(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);

            for (int32 HeightIndex = 0; HeightIndex < SafeVerticalSamples; ++HeightIndex)
            {
                const float HeightAlpha = SafeVerticalSamples == 1
                    ? 0.5f
                    : static_cast<float>(HeightIndex) / static_cast<float>(SafeVerticalSamples - 1);
                const float ZOffset = OutlineSampleHeight + (HeightAlpha - 0.5f) * OutlineVerticalSpan;

                const FVector Start = Wave.Origin + FVector(0.0f, 0.0f, ZOffset);
                const FVector End = Start + Direction * Wave.CurrentRadius;

                FHitResult Hit;
                FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EcholocationOutline), false);
                const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);
                if (!bHit) continue;

                const FVector HitPoint = Hit.ImpactPoint;
                const FVector Vertical = FVector::UpVector * (OutlineStrokeLength * 0.5f);
                const FVector Tangent = FVector::CrossProduct(Hit.ImpactNormal, FVector::UpVector).GetSafeNormal() * (OutlineStrokeLength * 0.5f);
                const FVector NormalStroke = Hit.ImpactNormal * (OutlineStrokeLength * 0.25f);

                DrawDebugLine(GetWorld(), HitPoint - Vertical, HitPoint + Vertical, OutlineColor, false, OutlineLifetime, 0, OutlineThickness);

                if (!Tangent.IsNearlyZero())
                {
                    DrawDebugLine(GetWorld(), HitPoint - Tangent, HitPoint + Tangent, OutlineColor, false, OutlineLifetime, 0, OutlineThickness);
                }

                DrawDebugLine(GetWorld(), HitPoint, HitPoint + NormalStroke, OutlineColor, false, OutlineLifetime, 0, OutlineThickness * 0.7f);
            }

            if (!bDrawHorizontalSurfaceOutlines) continue;

            for (int32 RadiusIndex = 1; RadiusIndex <= SafeSurfaceRadialSamples; ++RadiusIndex)
            {
                const float RadiusAlpha = static_cast<float>(RadiusIndex) / static_cast<float>(SafeSurfaceRadialSamples);
                const FVector SampleLocation = Wave.Origin + Direction * (Wave.CurrentRadius * RadiusAlpha);

                const FVector FloorStart = SampleLocation + FVector(0.0f, 0.0f, FloorTraceStartOffset);
                const FVector FloorEnd = SampleLocation - FVector(0.0f, 0.0f, FloorTraceDepth);
                const FVector CeilingStart = SampleLocation + FVector(0.0f, 0.0f, CeilingTraceStartOffset);
                const FVector CeilingEnd = SampleLocation + FVector(0.0f, 0.0f, CeilingTraceHeight);

                FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EcholocationHorizontalSurface), false);

                const auto DrawSurfaceMark = [this, &OutlineColor](const FHitResult& Hit)
                {
                    const FVector HitPoint = Hit.ImpactPoint;
                    const FVector Normal = Hit.ImpactNormal.GetSafeNormal();
                    const FVector AxisA = FVector::VectorPlaneProject(FVector::ForwardVector, Normal).GetSafeNormal() * (OutlineStrokeLength * 0.45f);
                    const FVector AxisB = FVector::VectorPlaneProject(FVector::RightVector, Normal).GetSafeNormal() * (OutlineStrokeLength * 0.45f);
                    const FVector NormalStroke = Normal * (OutlineStrokeLength * 0.2f);

                    if (!AxisA.IsNearlyZero())
                    {
                        DrawDebugLine(GetWorld(), HitPoint - AxisA, HitPoint + AxisA, OutlineColor, false, OutlineLifetime, 0, OutlineThickness);
                    }

                    if (!AxisB.IsNearlyZero())
                    {
                        DrawDebugLine(GetWorld(), HitPoint - AxisB, HitPoint + AxisB, OutlineColor, false, OutlineLifetime, 0, OutlineThickness);
                    }

                    DrawDebugLine(GetWorld(), HitPoint, HitPoint + NormalStroke, OutlineColor, false, OutlineLifetime, 0, OutlineThickness * 0.7f);
                };

                FHitResult FloorHit;
                if (GetWorld()->LineTraceSingleByChannel(FloorHit, FloorStart, FloorEnd, ECC_Visibility, QueryParams))
                {
                    DrawSurfaceMark(FloorHit);
                }

                FHitResult CeilingHit;
                if (GetWorld()->LineTraceSingleByChannel(CeilingHit, CeilingStart, CeilingEnd, ECC_Visibility, QueryParams))
                {
                    DrawSurfaceMark(CeilingHit);
                }
            }
        }
    }
}

FColor USoundWaveManager::GetWaveColor(const FSoundWaveRipple& Wave) const
{
    if (Wave.Priority >= 100)
    {
        return FColor(210, 210, 210);
    }

    if (Wave.Priority >= 50)
    {
        return FColor(190, 190, 190);
    }

    return FColor(165, 165, 165);
}
