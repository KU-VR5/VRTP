#include "MazeGameFlowManager.h"
#include "EscapeZone.h"
#include "GhostCharacter.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Framework/Application/SlateApplication.h"

class SMazeGameOverWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMazeGameOverWidget) {}
	SLATE_ARGUMENT(TWeakObjectPtr<class AMazeGameFlowManager>, FlowManager)
	SLATE_ARGUMENT(TWeakObjectPtr<UWorld>, World)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		FlowManager = InArgs._FlowManager;
		WidgetWorld = InArgs._World;

		FSlateFontInfo TitleFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		TitleFont.Size = 48;

		FSlateFontInfo SubFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		SubFont.Size = 18;

		FSlateFontInfo ButtonFont = FCoreStyle::Get().GetFontStyle("NormalFont");
		ButtonFont.Size = 20;

		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SColorBlock)
				.Color(FLinearColor(0.05f, 0.0f, 0.0f, 0.9f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.f)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("GameOver", "GameOverText", "GAME OVER"))
					.Font(TitleFont)
					.ColorAndOpacity(FLinearColor::Red)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.f)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("GameOver", "CaughtText", "The Ghost Caught You in the Dark..."))
					.Font(SubFont)
					.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.f))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(30.f)
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.OnClicked(this, &SMazeGameOverWidget::OnRestartClicked)
					.ContentPadding(FMargin(40.f, 15.f))
					.Content()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("GameOver", "RestartText", "RESTART GAME (Press R)"))
						.Font(ButtonFont)
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
		];
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		FKey Key = InKeyEvent.GetKey();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Key Pressed on Game Over: %s"), *Key.ToString()));
		}

		if (Key == EKeys::R || Key == EKeys::SpaceBar || Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom)
		{
			TriggerRestart();
			return FReply::Handled();
		}
		return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
	}

	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

private:
	TWeakObjectPtr<class AMazeGameFlowManager> FlowManager;
	TWeakObjectPtr<UWorld> WidgetWorld;

	void TriggerRestart()
	{
		UWorld* WorldContext = WidgetWorld.Get();
		if (!WorldContext && GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.World())
				{
					WorldContext = Context.World();
					break;
				}
			}
		}

		// Remove ourselves from the persistent viewport client immediately
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(AsShared());
		}

		if (WorldContext)
		{
			// Hide mouse cursor and reset input mode back to game only
			APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0);
			if (PC)
			{
				PC->bShowMouseCursor = false;
				FInputModeGameOnly InputMode;
				PC->SetInputMode(InputMode);
			}

			const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(WorldContext, true));
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Widget Triggered Restart! Map: %s"), *CurrentLevelName.ToString()));
			}
			UGameplayStatics::OpenLevel(WorldContext, CurrentLevelName, true);
		}
	}

	FReply OnRestartClicked()
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Restart Button Clicked!"));
		}
		TriggerRestart();
		return FReply::Handled();
	}
};


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
	UWorld* World = GetWorld();
	if (!World) return;

	// Remove persistent Slate widget immediately
	if (GEngine && GEngine->GameViewport && GameOverWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(GameOverWidget.ToSharedRef());
		GameOverWidget.Reset();
	}

	// Hide mouse cursor and reset input mode back to game only
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC)
	{
		PC->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	const FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(World, true));
	
	UE_LOG(LogTemp, Warning, TEXT("AMazeGameFlowManager::RestartLevel - Loading Map: %s"), *CurrentLevelName.ToString());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Restarting Level: %s"), *CurrentLevelName.ToString()));
	}

	UGameplayStatics::OpenLevel(World, CurrentLevelName, true);
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

	if (NewState == EMazeGameFlowState::Lost)
	{
		// Spawn Slate Game Over UI directly
		if (GEngine && GEngine->GameViewport)
		{
			SAssignNew(GameOverWidget, SMazeGameOverWidget)
				.FlowManager(this)
				.World(GetWorld());

			GEngine->GameViewport->AddViewportWidgetContent(GameOverWidget.ToSharedRef());

			// Set Keyboard Focus so key overrides work immediately!
			FSlateApplication::Get().SetKeyboardFocus(GameOverWidget.ToSharedRef());
		}

		// Disable player inputs and show mouse cursor for clicking
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			PC->SetIgnoreMoveInput(true);
			PC->SetIgnoreLookInput(true);
			PC->bShowMouseCursor = true;
			
			FInputModeGameAndUI InputModeData;
			PC->SetInputMode(InputModeData);
		}

		// Enable input on the FlowManager itself so the R/Enter/Space key binding works!
		EnableInput(PC);
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AMazeGameFlowManager::RestartLevel);
			InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AMazeGameFlowManager::RestartLevel);
			InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AMazeGameFlowManager::RestartLevel);
			InputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, this, &AMazeGameFlowManager::RestartLevel);
		}
		return;
	}

	TSubclassOf<UUserWidget> WidgetClass = nullptr;
	switch (NewState)
	{
	case EMazeGameFlowState::MainMenu:
		WidgetClass = MainMenuWidgetClass;
		break;
	case EMazeGameFlowState::Won:
		WidgetClass = WinWidgetClass;
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

	if (GEngine && GEngine->GameViewport && GameOverWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(GameOverWidget.ToSharedRef());
		GameOverWidget.Reset();
	}
}
