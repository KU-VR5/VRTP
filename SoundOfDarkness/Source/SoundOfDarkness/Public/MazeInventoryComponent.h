#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MazeInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuseCountChanged, int32, FuseCount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOUNDOFDARKNESS_API UMazeInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMazeInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Maze Inventory")
	void AddFuse(int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Maze Inventory")
	bool ConsumeFuse(int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Maze Inventory")
	int32 GetFuseCount() const { return FuseCount; }

	UPROPERTY(BlueprintAssignable, Category = "Maze Inventory")
	FOnFuseCountChanged OnFuseCountChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "Maze Inventory")
	int32 FuseCount = 0;
};
