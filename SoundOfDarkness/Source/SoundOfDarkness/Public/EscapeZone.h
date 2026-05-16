#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeZone.generated.h"

class UBoxComponent;
class AExitDoor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEscaped, AActor*, PlayerActor);

UCLASS()
class SOUNDOFDARKNESS_API AEscapeZone : public AActor
{
	GENERATED_BODY()

public:
	AEscapeZone();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* EscapeTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escape")
	AExitDoor* RequiredOpenDoor = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Escape")
	FOnPlayerEscaped OnPlayerEscaped;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
