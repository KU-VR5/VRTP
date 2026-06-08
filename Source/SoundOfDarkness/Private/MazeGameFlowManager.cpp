#include "MazeGameFlowManager.h"
#include "EscapeZone.h"
#include "GhostCharacter.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

AMazeGameFlowManager::AMazeGameFlowManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMazeGameFlowManager::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoBindLevelActors)
	{
		BindLevelActors();
	}

	CurrentState = bStartInMenu ? EMazeGameFlowState::MainMenu : EMazeGameFlowState::Playing;
	ShowWidgetForState(CurrentState);
	OnStateChanged.Broadcast(CurrentState);
}

void AMazeGameFlowManager::StartGame()
{
	SetState(EMazeGameFlowState::Playing);
}

void AMazeGameFlowManager::WinGame()
{
	SetState(EMazeGameFlowState::Won);
}

void AMazeGameFlowManager::LoseGame()
{
	SetState(EMazeGameFlowState::Lost);
}

void AMazeGameFlowManager::RestartLevel()
{
	if (!GetWorld()) return;

	const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this));
	UGameplayStatics::OpenLevel(this, CurrentLevelName);
}

void AMazeGameFlowManager::SetState(EMazeGameFlowState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	ShowWidgetForState(CurrentState);
	OnStateChanged.Broadcast(CurrentState);
}

void AMazeGameFlowManager::BindLevelActors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AEscapeZone> It(World); It; ++It)
	{
		It->OnPlayerEscaped.AddUniqueDynamic(this, &AMazeGameFlowManager::HandlePlayerEscaped);
	}

	for (TActorIterator<AGhostCharacter> It(World); It; ++It)
	{
		It->OnGhostCaughtPlayer.AddUniqueDynamic(this, &AMazeGameFlowManager::HandleGhostCaughtPlayer);
	}
}

void AMazeGameFlowManager::HandlePlayerEscaped(AActor* PlayerActor)
{
	WinGame();
}

void AMazeGameFlowManager::HandleGhostCaughtPlayer(APawn* PlayerPawn)
{
	LoseGame();
}

void AMazeGameFlowManager::ShowWidgetForState(EMazeGameFlowState NewState)
{
	ClearCurrentWidget();

	TSubclassOf<UUserWidget> WidgetClass = nullptr;
	switch (NewState)
	{
	case EMazeGameFlowState::MainMenu:
		WidgetClass = MainMenuWidgetClass;
		break;
	case EMazeGameFlowState::Won:
		WidgetClass = WinWidgetClass;
		break;
	case EMazeGameFlowState::Lost:
		WidgetClass = LoseWidgetClass;
		break;
	default:
		break;
	}

	if (!WidgetClass) return;

	CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
	if (CurrentWidget)
	{
		CurrentWidget->AddToViewport();
	}
}

void AMazeGameFlowManager::ClearCurrentWidget()
{
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromParent();
		CurrentWidget = nullptr;
	}
}
