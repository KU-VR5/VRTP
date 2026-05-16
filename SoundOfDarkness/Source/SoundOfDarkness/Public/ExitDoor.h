#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExitDoor.generated.h"

class UStaticMeshComponent;

UCLASS()
class SOUNDOFDARKNESS_API AExitDoor : public AActor
{
	GENERATED_BODY()

public:
	AExitDoor();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Exit Door")
	void OpenDoor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit Door")
	FVector OpenOffset = FVector(0.0f, 0.0f, 250.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit Door")
	float OpenSpeed = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exit Door")
	bool bIsOpen = false;

protected:
	virtual void BeginPlay() override;

private:
	FVector ClosedLocation;
	FVector OpenLocation;
};
