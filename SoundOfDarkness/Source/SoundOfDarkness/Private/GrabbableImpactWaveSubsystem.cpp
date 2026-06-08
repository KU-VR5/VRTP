#include "GrabbableImpactWaveSubsystem.h"
#include "SoundWaveManager.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

void UGrabbableImpactWaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ScanAndBindGrabbables();
}

void UGrabbableImpactWaveSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastScan += DeltaTime;
	if (TimeSinceLastScan >= ScanInterval)
	{
		TimeSinceLastScan = 0.0f;
		ScanAndBindGrabbables();
	}
}

TStatId UGrabbableImpactWaveSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UGrabbableImpactWaveSubsystem, STATGROUP_Tickables);
}

void UGrabbableImpactWaveSubsystem::ScanAndBindGrabbables()
{
	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown) return;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsGrabbableActor(Actor)) continue;

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* Component : PrimitiveComponents)
		{
			if (!Component || BoundComponents.Contains(Component)) continue;

			ConfigureImpactComponent(Component);
			Component->OnComponentHit.AddUniqueDynamic(this, &UGrabbableImpactWaveSubsystem::HandleGrabbableHit);
			BoundComponents.Add(Component);
			UE_LOG(LogTemp, Warning, TEXT("Bound grabbable impact wave component: %s.%s"), *Actor->GetName(), *Component->GetName());
		}
	}
}

bool UGrabbableImpactWaveSubsystem::IsGrabbableActor(const AActor* Actor) const
{
	if (!Actor) return false;

	const FString ActorName = Actor->GetName();
	const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();

	return ActorName.Contains(TEXT("GrabActor"))
		|| ClassName.Contains(TEXT("BP_Grabbable_SmallCube"));
}

bool UGrabbableImpactWaveSubsystem::IsMazeImpactTarget(const AActor* Actor, const UPrimitiveComponent* Component) const
{
	if (!Actor && !Component) return false;

	if (Component && Component->GetCollisionObjectType() == ECC_WorldStatic)
	{
		return true;
	}

	const FString ActorName = Actor ? Actor->GetName() : FString();
	const FString ComponentName = Component ? Component->GetName() : FString();
	const FString CombinedName = ActorName + TEXT(" ") + ComponentName;

	return CombinedName.Contains(TEXT("Maze"))
		|| CombinedName.Contains(TEXT("Wall"))
		|| CombinedName.Contains(TEXT("Floor"))
		|| CombinedName.Contains(TEXT("Path"))
		|| CombinedName.Contains(TEXT("SM_MERGED"));
}

void UGrabbableImpactWaveSubsystem::ConfigureImpactComponent(UPrimitiveComponent* Component)
{
	if (!Component) return;

	Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Component->SetCollisionObjectType(ECC_PhysicsBody);
	Component->SetCollisionResponseToAllChannels(ECR_Block);
	Component->SetNotifyRigidBodyCollision(true);
}

void UGrabbableImpactWaveSubsystem::HandleGrabbableHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HitComponent || !OtherActor || OtherActor == HitComponent->GetOwner()) return;
	if (!IsMazeImpactTarget(OtherActor, OtherComp)) return;

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float* LastImpactTime = LastImpactTimes.Find(HitComponent);
	if (LastImpactTime && CurrentTime - *LastImpactTime < ImpactCooldown) return;

	const float ImpactSpeed = FMath::Max(HitComponent->GetComponentVelocity().Size(), NormalImpulse.Size() * 0.01f);
	if (ImpactSpeed < MinimumImpactSpeed) return;

	LastImpactTimes.FindOrAdd(HitComponent) = CurrentTime;

	if (USoundWaveManager* SoundManager = GetWorld()->GetSubsystem<USoundWaveManager>())
	{
		const FVector ImpactPoint(Hit.ImpactPoint);
		const FVector WaveLocation = ImpactPoint.IsNearlyZero() ? HitComponent->GetComponentLocation() : ImpactPoint;
		SoundManager->SpawnSoundWave(WaveLocation, ImpactWaveRadius, ImpactWaveSpeed, ImpactWaveIntensity, 50);
		const FString WaveLocationString = WaveLocation.ToString();
		const FString OtherActorName = OtherActor->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Grabbable impact wave spawned at %s after hit with %s"), *WaveLocationString, *OtherActorName);
	}
}
