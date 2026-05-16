#include "MazeInventoryComponent.h"

UMazeInventoryComponent::UMazeInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMazeInventoryComponent::AddFuse(int32 Amount)
{
	if (Amount <= 0) return;
	FuseCount += Amount;
	OnFuseCountChanged.Broadcast(FuseCount);
}

bool UMazeInventoryComponent::ConsumeFuse(int32 Amount)
{
	if (Amount <= 0 || FuseCount < Amount) return false;
	FuseCount -= Amount;
	OnFuseCountChanged.Broadcast(FuseCount);
	return true;
}
