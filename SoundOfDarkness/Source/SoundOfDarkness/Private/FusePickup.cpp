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

	// Initialize and attach the 5 parts
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	MeshLight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshLight"));
	MeshLight->SetupAttachment(RootComponent);
	MeshLight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshLight->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	MeshPoles = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshPoles"));
	MeshPoles->SetupAttachment(RootComponent);
	MeshPoles->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshPoles->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	MeshGasket = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshGasket"));
	MeshGasket->SetupAttachment(RootComponent);
	MeshGasket->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshGasket->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	MeshGlass = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshGlass"));
	MeshGlass->SetupAttachment(RootComponent);
	MeshGlass->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshGlass->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	FuseLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FuseLight"));
	FuseLight->SetupAttachment(RootComponent);
	FuseLight->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	FuseLight->SetMobility(EComponentMobility::Movable);
	FuseLight->SetIntensity(FuseLightIntensity);
	FuseLight->SetAttenuationRadius(FuseLightRadius);
	FuseLight->SetLightColor(FuseLightColor);
	FuseLight->SetCastShadows(false);
	FuseLight->SetVisibility(bEnableFuseLight);

	// Load the 5 mesh files
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshCoverFinder(TEXT("/Game/Models/Fuse/fuse_cover1.fuse_cover1"));
	if (MeshCoverFinder.Succeeded())
	{
		Mesh->SetStaticMesh(MeshCoverFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshLightFinder(TEXT("/Game/Models/Fuse/fuse_light1.fuse_light1"));
	if (MeshLightFinder.Succeeded())
	{
		MeshLight->SetStaticMesh(MeshLightFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshPolesFinder(TEXT("/Game/Models/Fuse/fuse_poles1.fuse_poles1"));
	if (MeshPolesFinder.Succeeded())
	{
		MeshPoles->SetStaticMesh(MeshPolesFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshGasketFinder(TEXT("/Game/Models/Fuse/gasket1.gasket1"));
	if (MeshGasketFinder.Succeeded())
	{
		MeshGasket->SetStaticMesh(MeshGasketFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshGlassFinder(TEXT("/Game/Models/Fuse/glass1.glass1"));
	if (MeshGlassFinder.Succeeded())
	{
		MeshGlass->SetStaticMesh(MeshGlassFinder.Object);
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
