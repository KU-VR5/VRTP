#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightControllerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOUNDOFDARKNESS_API UFlashlightControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFlashlightControllerComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void BindInput();
	void OnCKeyPressed();

	bool bInputBound = false;
};
