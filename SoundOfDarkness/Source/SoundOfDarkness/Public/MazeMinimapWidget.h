#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MazeMinimapWidget.generated.h"

UCLASS()
class SOUNDOFDARKNESS_API UMazeMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float MapSize = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float ScreenMargin = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float MinWorldExtent = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	int32 MaxMazeInstancesToDraw = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	bool bDrawAllWorldStaticGeometry = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float LargeFlatPanelMinExtent = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float MaxFlatPanelHeight = 120.0f;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetMazeRootActors(const TArray<AActor*>& InMazeRootActors);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetExitMarkerActors(const TArray<AActor*>& InExitMarkerActors);

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	struct FMazeMinimapBounds
	{
		FVector2D Min = FVector2D(FLT_MAX, FLT_MAX);
		FVector2D Max = FVector2D(-FLT_MAX, -FLT_MAX);

		void Include(const FVector& Location);
		void IncludeBox(const FBoxSphereBounds& Bounds);
		bool IsValid() const;
	};

	FVector2D WorldToMap(const FVector& Location, const FMazeMinimapBounds& Bounds, const FVector2D& Origin, float Size) const;
	void GatherMazeComponentsFromActor(AActor* Actor, TArray<UInstancedStaticMeshComponent*>& OutInstancedComponents, TArray<UPrimitiveComponent*>& OutPrimitiveComponents, FMazeMinimapBounds& OutBounds, TSet<const AActor*>& VisitedActors) const;
	void DrawLine(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, float Thickness = 1.0f) const;
	void DrawRect(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Min, const FVector2D& Max, const FLinearColor& Color, float Thickness = 1.0f) const;
	void DrawFilledRect(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Min, const FVector2D& Max, const FLinearColor& Color) const;
	void DrawPoint(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, const FVector2D& Center, const FLinearColor& Color, float Radius = 5.0f) const;
	bool IsDrawablePlanarBounds(const FVector& Extent) const;
	bool IsLargeFloorOrCeilingBounds(const FVector& Extent) const;
	bool ShouldDrawMazeComponent(const UPrimitiveComponent* Component) const;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> MazeRootActors;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ExitMarkerActors;
};
