#include "MazeMinimapWidget.h"
#include "ExitDoor.h"
#include "FusePanel.h"
#include "FusePickup.h"
#include "GhostCharacter.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void UMazeMinimapWidget::FMazeMinimapBounds::Include(const FVector& Location)
{
	Min.X = FMath::Min(Min.X, Location.X);
	Min.Y = FMath::Min(Min.Y, Location.Y);
	Max.X = FMath::Max(Max.X, Location.X);
	Max.Y = FMath::Max(Max.Y, Location.Y);
}

void UMazeMinimapWidget::FMazeMinimapBounds::IncludeBox(const FBoxSphereBounds& Bounds)
{
	Include(Bounds.Origin - Bounds.BoxExtent);
	Include(Bounds.Origin + Bounds.BoxExtent);
}

bool UMazeMinimapWidget::FMazeMinimapBounds::IsValid() const
{
	return Min.X <= Max.X && Min.Y <= Max.Y;
}

void UMazeMinimapWidget::SetMazeRootActors(const TArray<AActor*>& InMazeRootActors)
{
	MazeRootActors.Reset();
	for (AActor* Actor : InMazeRootActors)
	{
		if (Actor)
		{
			MazeRootActors.Add(Actor);
		}
	}
}

void UMazeMinimapWidget::SetExitMarkerActors(const TArray<AActor*>& InExitMarkerActors)
{
	ExitMarkerActors.Reset();
	for (AActor* Actor : InExitMarkerActors)
	{
		if (Actor)
		{
			ExitMarkerActors.Add(Actor);
		}
	}
}

int32 UMazeMinimapWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 ResultLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	UWorld* World = GetWorld();
	if (!World)
	{
		return ResultLayer;
	}

	FMazeMinimapBounds Bounds;
	TArray<UInstancedStaticMeshComponent*> InstancedComponents;
	TArray<UPrimitiveComponent*> PrimitiveComponents;

	bool bUsedManualMazeRoots = false;
	TSet<const AActor*> VisitedMazeActors;
	for (AActor* MazeRootActor : MazeRootActors)
	{
		if (!MazeRootActor || MazeRootActor->IsPendingKillPending() || MazeRootActor->GetWorld() != World) continue;

		bUsedManualMazeRoots = true;
		GatherMazeComponentsFromActor(MazeRootActor, InstancedComponents, PrimitiveComponents, Bounds, VisitedMazeActors);
	}

	if (!bUsedManualMazeRoots)
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			GatherMazeComponentsFromActor(Actor, InstancedComponents, PrimitiveComponents, Bounds, VisitedMazeActors);
		}
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			PlayerPawn = PlayerController->GetPawn();
		}
	}
	if (PlayerPawn)
	{
		Bounds.Include(PlayerPawn->GetActorLocation());
	}

	TArray<AActor*> ActorsToMark;
	UGameplayStatics::GetAllActorsOfClass(this, AGhostCharacter::StaticClass(), ActorsToMark);
	for (AActor* Actor : ActorsToMark)
	{
		if (Actor) Bounds.Include(Actor->GetActorLocation());
	}
	ActorsToMark.Reset();
	UGameplayStatics::GetAllActorsOfClass(this, AFusePickup::StaticClass(), ActorsToMark);
	for (AActor* Actor : ActorsToMark)
	{
		if (Actor) Bounds.Include(Actor->GetActorLocation());
	}
	ActorsToMark.Reset();
	UGameplayStatics::GetAllActorsOfClass(this, AFusePanel::StaticClass(), ActorsToMark);
	for (AActor* Actor : ActorsToMark)
	{
		if (Actor) Bounds.Include(Actor->GetActorLocation());
	}
	ActorsToMark.Reset();
	UGameplayStatics::GetAllActorsOfClass(this, AExitDoor::StaticClass(), ActorsToMark);
	for (AActor* Actor : ActorsToMark)
	{
		if (Actor) Bounds.Include(Actor->GetActorLocation());
	}
	for (AActor* Actor : ExitMarkerActors)
	{
		if (Actor && !Actor->IsPendingKillPending() && Actor->GetWorld() == World)
		{
			Bounds.Include(Actor->GetActorLocation());
		}
	}

	if (!Bounds.IsValid())
	{
		return ResultLayer;
	}

	const FVector2D WorldCenter = (Bounds.Min + Bounds.Max) * 0.5f;
	const FVector2D WorldExtent = FVector2D(
		FMath::Max((Bounds.Max.X - Bounds.Min.X) * 0.5f, MinWorldExtent),
		FMath::Max((Bounds.Max.Y - Bounds.Min.Y) * 0.5f, MinWorldExtent)
	);
	Bounds.Min = WorldCenter - WorldExtent;
	Bounds.Max = WorldCenter + WorldExtent;

	const FVector2D ViewportSize = AllottedGeometry.GetLocalSize();
	const float SafeMapSize = FMath::Min(MapSize, FMath::Min(ViewportSize.X, ViewportSize.Y) - ScreenMargin * 2.0f);
	const FVector2D MapOrigin(ViewportSize.X - SafeMapSize - ScreenMargin, ScreenMargin);
	const FVector2D MapMax = MapOrigin + FVector2D(SafeMapSize, SafeMapSize);

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		ResultLayer + 1,
		AllottedGeometry.ToPaintGeometry(MapOrigin, FVector2D(SafeMapSize, SafeMapSize)),
		FCoreStyle::Get().GetBrush("WhiteBrush"),
		ESlateDrawEffect::None,
		FLinearColor(0.01f, 0.01f, 0.012f, 0.82f)
	);

	DrawRect(OutDrawElements, AllottedGeometry, ResultLayer + 2, MapOrigin, MapMax, FLinearColor(0.55f, 0.55f, 0.55f, 0.9f), 2.0f);

	int32 DrawnInstances = 0;
	for (const UInstancedStaticMeshComponent* Component : InstancedComponents)
	{
		if (!Component || !Component->GetStaticMesh()) continue;

		const int32 InstanceCount = Component->GetInstanceCount();
		const FBoxSphereBounds MeshBounds = Component->GetStaticMesh()->GetBounds();
		for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount && DrawnInstances < MaxMazeInstancesToDraw; ++InstanceIndex)
		{
			FTransform InstanceTransform;
			if (!Component->GetInstanceTransform(InstanceIndex, InstanceTransform, true)) continue;

			const FVector WorldLocation = InstanceTransform.GetLocation();
			const FVector ScaledExtent = MeshBounds.BoxExtent * InstanceTransform.GetScale3D().GetAbs();
			if (!IsDrawablePlanarBounds(ScaledExtent)) continue;

			const FVector2D MinPoint = WorldToMap(WorldLocation - ScaledExtent, Bounds, MapOrigin, SafeMapSize);
			const FVector2D MaxPoint = WorldToMap(WorldLocation + ScaledExtent, Bounds, MapOrigin, SafeMapSize);
			DrawFilledRect(OutDrawElements, AllottedGeometry, ResultLayer + 3, MinPoint, MaxPoint, FLinearColor(0.78f, 0.78f, 0.78f, 0.78f));
			DrawRect(OutDrawElements, AllottedGeometry, ResultLayer + 4, MinPoint, MaxPoint, FLinearColor(0.95f, 0.95f, 0.95f, 0.9f), 1.0f);
			++DrawnInstances;
		}
	}

	for (const UPrimitiveComponent* Component : PrimitiveComponents)
	{
		if (!Component) continue;
		const FVector2D MinPoint = WorldToMap(Component->Bounds.Origin - Component->Bounds.BoxExtent, Bounds, MapOrigin, SafeMapSize);
		const FVector2D MaxPoint = WorldToMap(Component->Bounds.Origin + Component->Bounds.BoxExtent, Bounds, MapOrigin, SafeMapSize);
		DrawFilledRect(OutDrawElements, AllottedGeometry, ResultLayer + 3, MinPoint, MaxPoint, FLinearColor(0.72f, 0.72f, 0.72f, 0.65f));
		DrawRect(OutDrawElements, AllottedGeometry, ResultLayer + 4, MinPoint, MaxPoint, FLinearColor(0.94f, 0.94f, 0.94f, 0.85f), 1.0f);
	}

	if (PlayerPawn)
	{
		DrawPoint(OutDrawElements, AllottedGeometry, ResultLayer + 6, WorldToMap(PlayerPawn->GetActorLocation(), Bounds, MapOrigin, SafeMapSize), FLinearColor(0.1f, 0.9f, 0.25f, 1.0f), 7.0f);
	}

	ActorsToMark.Reset();
	UGameplayStatics::GetAllActorsOfClass(this, AGhostCharacter::StaticClass(), ActorsToMark);
	for (AActor* Actor : ActorsToMark)
	{
		if (Actor) DrawPoint(OutDrawElements, AllottedGeometry, ResultLayer + 6, WorldToMap(Actor->GetActorLocation(), Bounds, MapOrigin, SafeMapSize), FLinearColor(1.0f, 0.1f, 0.1f, 1.0f), 7.0f);
	}

	ActorsToMark.Reset();
	UGameplayStatics::GetAllActorsOfClass(this, AFusePickup::StaticClass(), ActorsToMark);
	for (AActor* Actor : ActorsToMark)
	{
		if (Actor) DrawPoint(OutDrawElements, AllottedGeometry, ResultLayer + 6, WorldToMap(Actor->GetActorLocation(), Bounds, MapOrigin, SafeMapSize), FLinearColor(1.0f, 0.85f, 0.05f, 1.0f), 5.5f);
	}

	ActorsToMark.Reset();
	UGameplayStatics::GetAllActorsOfClass(this, AFusePanel::StaticClass(), ActorsToMark);
	for (AActor* Actor : ActorsToMark)
	{
		if (Actor) DrawPoint(OutDrawElements, AllottedGeometry, ResultLayer + 6, WorldToMap(Actor->GetActorLocation(), Bounds, MapOrigin, SafeMapSize), FLinearColor(0.2f, 0.45f, 1.0f, 1.0f), 6.0f);
	}

	ActorsToMark.Reset();
	UGameplayStatics::GetAllActorsOfClass(this, AExitDoor::StaticClass(), ActorsToMark);
	for (AActor* Actor : ActorsToMark)
	{
		if (Actor) DrawPoint(OutDrawElements, AllottedGeometry, ResultLayer + 6, WorldToMap(Actor->GetActorLocation(), Bounds, MapOrigin, SafeMapSize), FLinearColor(0.0f, 0.95f, 1.0f, 1.0f), 6.0f);
	}
	for (AActor* Actor : ExitMarkerActors)
	{
		if (Actor && !Actor->IsPendingKillPending() && Actor->GetWorld() == World)
		{
			DrawPoint(OutDrawElements, AllottedGeometry, ResultLayer + 6, WorldToMap(Actor->GetActorLocation(), Bounds, MapOrigin, SafeMapSize), FLinearColor(0.0f, 0.95f, 1.0f, 1.0f), 8.0f);
		}
	}

	return ResultLayer + 7;
}

FVector2D UMazeMinimapWidget::WorldToMap(const FVector& Location, const FMazeMinimapBounds& Bounds, const FVector2D& Origin, float Size) const
{
	const float AlphaX = (Location.X - Bounds.Min.X) / FMath::Max(Bounds.Max.X - Bounds.Min.X, 1.0f);
	const float AlphaY = (Location.Y - Bounds.Min.Y) / FMath::Max(Bounds.Max.Y - Bounds.Min.Y, 1.0f);
	return Origin + FVector2D(AlphaX * Size, (1.0f - AlphaY) * Size);
}

void UMazeMinimapWidget::GatherMazeComponentsFromActor(AActor* Actor, TArray<UInstancedStaticMeshComponent*>& OutInstancedComponents, TArray<UPrimitiveComponent*>& OutPrimitiveComponents, FMazeMinimapBounds& OutBounds, TSet<const AActor*>& VisitedActors) const
{
	if (!Actor || Actor->IsPendingKillPending()) return;
	if (VisitedActors.Contains(Actor)) return;
	VisitedActors.Add(Actor);

	TArray<UPrimitiveComponent*> ActorComponents;
	Actor->GetComponents<UPrimitiveComponent>(ActorComponents);
	for (UPrimitiveComponent* Component : ActorComponents)
	{
		if (!Component || !ShouldDrawMazeComponent(Component)) continue;

		if (UInstancedStaticMeshComponent* InstancedComponent = Cast<UInstancedStaticMeshComponent>(Component))
		{
			OutInstancedComponents.Add(InstancedComponent);
			if (const UStaticMesh* StaticMesh = InstancedComponent->GetStaticMesh())
			{
				const FBoxSphereBounds MeshBounds = StaticMesh->GetBounds();
				const int32 InstanceCount = InstancedComponent->GetInstanceCount();
				for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount && InstanceIndex < MaxMazeInstancesToDraw; ++InstanceIndex)
				{
					FTransform InstanceTransform;
					if (!InstancedComponent->GetInstanceTransform(InstanceIndex, InstanceTransform, true)) continue;

					const FVector ScaledExtent = MeshBounds.BoxExtent * InstanceTransform.GetScale3D().GetAbs();
					if (!IsDrawablePlanarBounds(ScaledExtent)) continue;

					const FVector WorldLocation = InstanceTransform.GetLocation();
					OutBounds.Include(WorldLocation - ScaledExtent);
					OutBounds.Include(WorldLocation + ScaledExtent);
				}
			}
		}
		else
		{
			if (!IsDrawablePlanarBounds(Component->Bounds.BoxExtent)) continue;
			OutPrimitiveComponents.Add(Component);
			OutBounds.IncludeBox(Component->Bounds);
		}
	}

	TArray<AActor*> AttachedActors;
	Actor->GetAttachedActors(AttachedActors);
	for (AActor* AttachedActor : AttachedActors)
	{
		GatherMazeComponentsFromActor(AttachedActor, OutInstancedComponents, OutPrimitiveComponents, OutBounds, VisitedActors);
	}
}

void UMazeMinimapWidget::DrawLine(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, float Thickness) const
{
	TArray<FVector2D> Points;
	Points.Add(Start);
	Points.Add(End);
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::None,
		Color,
		true,
		Thickness
	);
}

void UMazeMinimapWidget::DrawRect(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Min, const FVector2D& Max, const FLinearColor& Color, float Thickness) const
{
	const FVector2D RectMin(FMath::Min(Min.X, Max.X), FMath::Min(Min.Y, Max.Y));
	const FVector2D RectMax(FMath::Max(Min.X, Max.X), FMath::Max(Min.Y, Max.Y));
	DrawLine(OutDrawElements, AllottedGeometry, LayerId, FVector2D(RectMin.X, RectMin.Y), FVector2D(RectMax.X, RectMin.Y), Color, Thickness);
	DrawLine(OutDrawElements, AllottedGeometry, LayerId, FVector2D(RectMax.X, RectMin.Y), FVector2D(RectMax.X, RectMax.Y), Color, Thickness);
	DrawLine(OutDrawElements, AllottedGeometry, LayerId, FVector2D(RectMax.X, RectMax.Y), FVector2D(RectMin.X, RectMax.Y), Color, Thickness);
	DrawLine(OutDrawElements, AllottedGeometry, LayerId, FVector2D(RectMin.X, RectMax.Y), FVector2D(RectMin.X, RectMin.Y), Color, Thickness);
}

void UMazeMinimapWidget::DrawFilledRect(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Min, const FVector2D& Max, const FLinearColor& Color) const
{
	const FVector2D RectMin(FMath::Min(Min.X, Max.X), FMath::Min(Min.Y, Max.Y));
	const FVector2D RectMax(FMath::Max(Min.X, Max.X), FMath::Max(Min.Y, Max.Y));
	const FVector2D Size = FVector2D(
		FMath::Max(RectMax.X - RectMin.X, 1.5f),
		FMath::Max(RectMax.Y - RectMin.Y, 1.5f)
	);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(RectMin, Size),
		FCoreStyle::Get().GetBrush("WhiteBrush"),
		ESlateDrawEffect::None,
		Color
	);
}

void UMazeMinimapWidget::DrawPoint(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Center, const FLinearColor& Color, float Radius) const
{
	const FVector2D Min = Center - FVector2D(Radius, Radius);
	const FVector2D Size(Radius * 2.0f, Radius * 2.0f);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(Min, Size),
		FCoreStyle::Get().GetBrush("WhiteBrush"),
		ESlateDrawEffect::None,
		Color
	);
}

bool UMazeMinimapWidget::IsDrawablePlanarBounds(const FVector& Extent) const
{
	return !IsLargeFloorOrCeilingBounds(Extent);
}

bool UMazeMinimapWidget::IsLargeFloorOrCeilingBounds(const FVector& Extent) const
{
	const bool bLargeInBothPlanarAxes = Extent.X >= LargeFlatPanelMinExtent && Extent.Y >= LargeFlatPanelMinExtent;
	const bool bThinVertically = Extent.Z <= MaxFlatPanelHeight;
	return bLargeInBothPlanarAxes && bThinVertically;
}

bool UMazeMinimapWidget::ShouldDrawMazeComponent(const UPrimitiveComponent* Component) const
{
	if (!Component || !Component->IsRegistered() || !Component->IsVisible()) return false;

	const AActor* Owner = Component->GetOwner();
	if (!Owner) return false;

	if (Owner->IsA<APawn>() || Owner->IsA<AFusePickup>() || Owner->IsA<AFusePanel>() || Owner->IsA<AExitDoor>() || Owner->IsA<AGhostCharacter>())
	{
		return false;
	}

	const bool bSupportedMeshComponent = Component->IsA<UStaticMeshComponent>() || Component->IsA<UInstancedStaticMeshComponent>();
	if (!bSupportedMeshComponent) return false;

	if (Component->Mobility == EComponentMobility::Movable)
	{
		return false;
	}

	if (bDrawAllWorldStaticGeometry)
	{
		return true;
	}

	const FString Name = Component->GetName();
	const FString OwnerName = Owner->GetName();

	return Name.Contains(TEXT("Maze")) || Name.Contains(TEXT("Wall")) || Name.Contains(TEXT("Floor")) || Name.Contains(TEXT("Path")) ||
		Name.Contains(TEXT("Merged")) || Name.Contains(TEXT("Instanced")) ||
		OwnerName.Contains(TEXT("Maze")) || OwnerName.Contains(TEXT("Wall")) || OwnerName.Contains(TEXT("Floor")) || OwnerName.Contains(TEXT("Path")) ||
		OwnerName.Contains(TEXT("Merged")) || OwnerName.Contains(TEXT("Instanced"));
}
