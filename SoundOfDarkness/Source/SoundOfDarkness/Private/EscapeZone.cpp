#include "EscapeZone.h"
#include "Components/BoxComponent.h"
#include "ExitDoor.h"
#include "GameFramework/Pawn.h"

AEscapeZone::AEscapeZone()
{
	PrimaryActorTick.bCanEverTick = false;

	EscapeTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeTrigger"));
	RootComponent = EscapeTrigger;
	EscapeTrigger->SetBoxExtent(FVector(120.0f, 120.0f, 120.0f));
	EscapeTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EscapeTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	EscapeTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AEscapeZone::BeginPlay()
{
	Super::BeginPlay();
	EscapeTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEscapeZone::OnTriggerBeginOverlap);
}

void AEscapeZone::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<APawn>(OtherActor)) return;
	if (RequiredOpenDoor && !RequiredOpenDoor->bIsOpen) return;

	UE_LOG(LogTemp, Log, TEXT("Player escaped the maze."));
	OnPlayerEscaped.Broadcast(OtherActor);
}
