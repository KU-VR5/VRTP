#include "FusePickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "MazeInventoryComponent.h"

AFusePickup::AFusePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("PickupTrigger"));
	RootComponent = PickupTrigger;
	PickupTrigger->SetSphereRadius(45.0f);
	PickupTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFusePickup::BeginPlay()
{
	Super::BeginPlay();
	PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &AFusePickup::OnTriggerBeginOverlap);
}

void AFusePickup::CollectFuse(AActor* Collector)
{
	if (!Collector) return;

	if (UMazeInventoryComponent* Inventory = Collector->FindComponentByClass<UMazeInventoryComponent>())
	{
		Inventory->AddFuse(FuseAmount);
		if (bDestroyOnPickup)
		{
			Destroy();
		}
	}
}

void AFusePickup::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APawn>(OtherActor))
	{
		CollectFuse(OtherActor);
	}
}
