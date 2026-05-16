#include "ExitDoor.h"
#include "Components/StaticMeshComponent.h"

AExitDoor::AExitDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;
}

void AExitDoor::BeginPlay()
{
	Super::BeginPlay();
	ClosedLocation = GetActorLocation();
	OpenLocation = ClosedLocation + OpenOffset;
}

void AExitDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsOpen) return;

	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), OpenLocation, DeltaTime, OpenSpeed);
	SetActorLocation(NewLocation);
}

void AExitDoor::OpenDoor()
{
	bIsOpen = true;
}
