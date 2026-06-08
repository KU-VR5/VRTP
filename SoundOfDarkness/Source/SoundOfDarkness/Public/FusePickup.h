#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FusePickup.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPointLightComponent;

UCLASS()
class SOUNDOFDARKNESS_API AFusePickup : public AActor
{
	GENERATED_BODY()

public:
	AFusePickup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshPoles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshGasket;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshGlass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPointLightComponent* FuseLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* PickupTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse|Visual")
	bool bEnableFuseLight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse|Visual", meta = (EditCondition = "bEnableFuseLight", ClampMin = "0.0"))
	float FuseLightIntensity = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse|Visual", meta = (EditCondition = "bEnableFuseLight", ClampMin = "0.0"))
	float FuseLightRadius = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse|Visual", meta = (EditCondition = "bEnableFuseLight"))
	FLinearColor FuseLightColor = FLinearColor(1.0f, 0.78f, 0.08f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse")
	int32 FuseAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse")
	bool bDestroyOnPickup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuse")
	bool bCreateInventoryIfMissing = true;

	UFUNCTION(BlueprintCallable, Category = "Fuse")
	void CollectFuse(AActor* Collector);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
