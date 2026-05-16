#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FusePickup.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class SOUNDOFDARKNESS_API AFusePickup : public AActor
{
	GENERATED_BODY()

public:
	AFusePickup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* PickupTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse")
	int32 FuseAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse")
	bool bDestroyOnPickup = true;

	UFUNCTION(BlueprintCallable, Category = "Fuse")
	void CollectFuse(AActor* Collector);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
