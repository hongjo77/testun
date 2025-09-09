// Systems/CYInventoryComponent.cpp
#include "Systems/CYInventoryComponent.h"
#include "Item/CYItemBase.h"
#include "Net/UnrealNetwork.h"

UCYInventoryComponent::UCYInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 배열 크기 초기화
    Items.SetNum(MaxSlots);
}

void UCYInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION(UCYInventoryComponent, Items, COND_OwnerOnly);
}

void UCYInventoryComponent::ServerAddItem_Implementation(ACYItemBase* Item)
{
    if (!GetOwner()->HasAuthority() || !Item) return;

    int32 EmptySlot = GetFirstEmptySlotIndex();
    if (EmptySlot != -1)
    {
        Items[EmptySlot] = Item;
        OnInventoryUpdated.Broadcast();
        
        UE_LOG(LogTemp, Warning, TEXT("Added %s to slot %d"), *Item->GetName(), EmptySlot);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory is full!"));
    }
}

void UCYInventoryComponent::ServerRemoveItem_Implementation(int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority()) return;

    if (Items.IsValidIndex(SlotIndex) && Items[SlotIndex])
    {
        ACYItemBase* RemovedItem = Items[SlotIndex];
        Items[SlotIndex] = nullptr;
        OnInventoryUpdated.Broadcast();
        
        UE_LOG(LogTemp, Warning, TEXT("Removed %s from slot %d"), *RemovedItem->GetName(), SlotIndex);
    }
}

ACYItemBase* UCYInventoryComponent::GetItemAtSlot(int32 SlotIndex)
{
    if (Items.IsValidIndex(SlotIndex))
    {
        return Items[SlotIndex];
    }
    return nullptr;
}

bool UCYInventoryComponent::HasEmptySlot()
{
    return GetFirstEmptySlotIndex() != -1;
}

int32 UCYInventoryComponent::GetFirstEmptySlotIndex()
{
    for (int32 i = 0; i < Items.Num(); i++)
    {
        if (!Items[i])
        {
            return i;
        }
    }
    return -1;
}

int32 UCYInventoryComponent::GetItemCount(FGameplayTag ItemType)
{
    int32 Count = 0;
    for (ACYItemBase* Item : Items)
    {
        if (Item && Item->ItemType == ItemType)
        {
            Count++;
        }
    }
    return Count;
}

void UCYInventoryComponent::OnRep_Items()
{
    OnInventoryUpdated.Broadcast();
}