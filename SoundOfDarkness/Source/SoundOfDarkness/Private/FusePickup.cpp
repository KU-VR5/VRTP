#include "FusePickup.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "MazeInventoryComponent.h"
#include "UObject/ConstructorHelpers.h"

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
	Mesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));

	FuseLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FuseLight"));
	FuseLight->SetupAttachment(RootComponent);
	FuseLight->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	FuseLight->SetMobility(EComponentMobility::Movable);
	FuseLight->SetIntensity(FuseLightIntensity);
	FuseLight->SetAttenuationRadius(FuseLightRadius);
	FuseLight->SetLightColor(FuseLightColor);
	FuseLight->SetCastShadows(false);
	FuseLight->SetVisibility(bEnableFuseLight);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AFusePickup::BeginPlay()
{
	Super::BeginPlay();

	if (FuseLight)
	{
		FuseLight->SetVisibility(bEnableFuseLight);
		FuseLight->SetIntensity(FuseLightIntensity);
		FuseLight->SetAttenuationRadius(FuseLightRadius);
		FuseLight->SetLightColor(FuseLightColor);
	}

	PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &AFusePickup::OnTriggerBeginOverlap);
}

void AFusePickup::CollectFuse(AActor* Collector)
{
	if (!Collector) return;

	UMazeInventoryComponent* Inventory = Collector->FindComponentByClass<UMazeInventoryComponent>();
	if (!Inventory && bCreateInventoryIfMissing)
	{
		Inventory = NewObject<UMazeInventoryComponent>(Collector, UMazeInventoryComponent::StaticClass(), TEXT("MazeInventoryComponent"));
		if (Inventory)
		{
			Collector->AddInstanceComponent(Inventory);
			Inventory->RegisterComponent();
		}
	}

	if (Inventory)
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
