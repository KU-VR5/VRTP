#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FusePanel.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class AExitDoor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuseInserted, int32, InsertedFuseCount, int32, RequiredFuseCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFusePanelSolved);

UCLASS()
class SOUNDOFDARKNESS_API AFusePanel : public AActor
{
	GENERATED_BODY()

public:
	AFusePanel();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PanelMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* InsertTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse Panel")
	int32 RequiredFuseCount = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fuse Panel")
	int32 InsertedFuseCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse Panel")
	AExitDoor* LinkedExitDoor = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Fuse Panel")
	FOnFuseInserted OnFuseInserted;

	UPROPERTY(BlueprintAssignable, Category = "Fuse Panel")
	FOnFusePanelSolved OnFusePanelSolved;

	UFUNCTION(BlueprintCallable, Category = "Fuse Panel")
	bool TryInsertFuse(AActor* PlayerActor);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void SolvePanel();
};
