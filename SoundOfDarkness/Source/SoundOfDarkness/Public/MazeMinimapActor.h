#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeMinimapActor.generated.h"

class UMazeMinimapWidget;

UCLASS()
class SOUNDOFDARKNESS_API AMazeMinimapActor : public AActor
{
	GENERATED_BODY()

public:
	AMazeMinimapActor();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	bool bVisibleAtStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	TSubclassOf<UMazeMinimapWidget> MinimapWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	TArray<AActor*> MazeRootActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	TArray<AActor*> ExitMarkerActors;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ToggleMinimap();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetMinimapVisible(bool bNewVisible);

private:
	UPROPERTY()
	UMazeMinimapWidget* MinimapWidget = nullptr;

	bool bIsMinimapVisible = false;
};
