#include "FusePanel.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ExitDoor.h"
#include "GameFramework/Pawn.h"
#include "MazeInventoryComponent.h"

AFusePanel::AFusePanel()
{
	PrimaryActorTick.bCanEverTick = false;

	InsertTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("InsertTrigger"));
	RootComponent = InsertTrigger;
	InsertTrigger->SetSphereRadius(80.0f);
	InsertTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InsertTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	InsertTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));
	PanelMesh->SetupAttachment(RootComponent);
	PanelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AFusePanel::BeginPlay()
{
	Super::BeginPlay();
	InsertTrigger->OnComponentBeginOverlap.AddDynamic(this, &AFusePanel::OnTriggerBeginOverlap);
}

bool AFusePanel::TryInsertFuse(AActor* PlayerActor)
{
	if (!PlayerActor || InsertedFuseCount >= RequiredFuseCount) return false;

	UMazeInventoryComponent* Inventory = PlayerActor->FindComponentByClass<UMazeInventoryComponent>();
	if (!Inventory || !Inventory->ConsumeFuse(1)) return false;

	++InsertedFuseCount;
	OnFuseInserted.Broadcast(InsertedFuseCount, RequiredFuseCount);

	if (InsertedFuseCount >= RequiredFuseCount)
	{
		SolvePanel();
	}

	return true;
}

void AFusePanel::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APawn>(OtherActor))
	{
		TryInsertFuse(OtherActor);
	}
}

void AFusePanel::SolvePanel()
{
	OnFusePanelSolved.Broadcast();
	if (LinkedExitDoor)
	{
		LinkedExitDoor->OpenDoor();
	}
}
