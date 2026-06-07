#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeGameFlowManager.generated.h"

class AEscapeZone;
class AGhostCharacter;
class UUserWidget;

UENUM(BlueprintType)
enum class EMazeGameFlowState : uint8
{
	MainMenu UMETA(DisplayName = "Main Menu"),
	Playing UMETA(DisplayName = "Playing"),
	Won UMETA(DisplayName = "Won"),
	Lost UMETA(DisplayName = "Lost")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMazeGameFlowStateChanged, EMazeGameFlowState, NewState);

UCLASS()
class SOUNDOFDARKNESS_API AMazeGameFlowManager : public AActor
{
	GENERATED_BODY()

public:
	AMazeGameFlowManager();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	bool bStartInMenu = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	bool bAutoBindLevelActors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow|UI")
	TSubclassOf<UUserWidget> WinWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow|UI")
	TSubclassOf<UUserWidget> LoseWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow")
	EMazeGameFlowState CurrentState = EMazeGameFlowState::MainMenu;

	UPROPERTY(BlueprintAssignable, Category = "Game Flow")
	FOnMazeGameFlowStateChanged OnStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void WinGame();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void LoseGame();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void RestartLevel();

private:
	void SetState(EMazeGameFlowState NewState);
	void BindLevelActors();

	UFUNCTION()
	void HandlePlayerEscaped(AActor* PlayerActor);

	UFUNCTION()
	void HandleGhostCaughtPlayer(APawn* PlayerPawn);

	void ShowWidgetForState(EMazeGameFlowState NewState);
	void ClearCurrentWidget();

	UPROPERTY()
	UUserWidget* CurrentWidget = nullptr;

	TSharedPtr<SWidget> GameOverWidget;
};
