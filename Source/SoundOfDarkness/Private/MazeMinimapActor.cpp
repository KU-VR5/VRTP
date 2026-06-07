#include "MazeMinimapActor.h"
#include "MazeMinimapWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

AMazeMinimapActor::AMazeMinimapActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMazeMinimapActor::BeginPlay()
{
	Super::BeginPlay();

	if (!MinimapWidgetClass)
	{
		MinimapWidgetClass = UMazeMinimapWidget::StaticClass();
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PlayerController)
	{
		EnableInput(PlayerController);
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::M, IE_Pressed, this, &AMazeMinimapActor::ToggleMinimap);
		}

		MinimapWidget = CreateWidget<UMazeMinimapWidget>(PlayerController, MinimapWidgetClass);
		if (MinimapWidget)
		{
			MinimapWidget->SetMazeRootActors(MazeRootActors);
			MinimapWidget->SetExitMarkerActors(ExitMarkerActors);
			MinimapWidget->AddToViewport(50);
		}
	}

	SetMinimapVisible(bVisibleAtStart);
}

void AMazeMinimapActor::ToggleMinimap()
{
	SetMinimapVisible(!bIsMinimapVisible);
}

void AMazeMinimapActor::SetMinimapVisible(bool bNewVisible)
{
	bIsMinimapVisible = bNewVisible;
	if (MinimapWidget)
	{
		MinimapWidget->SetVisibility(bIsMinimapVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
