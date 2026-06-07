#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KeyboardMovementComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOUNDOFDARKNESS_API UKeyboardMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UKeyboardMovementComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Movement")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Movement")
	float LookSpeed = 45.0f;
};
