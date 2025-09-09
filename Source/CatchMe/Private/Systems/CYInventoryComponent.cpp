// Systems/CYInventoryComponent.cpp
#include "Systems/CYInventoryComponent.h"
#include "Item/CYItemBase.h"
#include "Item/CYTrapBase.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerCharacter.h"
#include "Weapon/CYWeaponBase.h"

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

void UCYInventoryComponent::SelectSlot(int32 SlotIndex)
{
    if (SlotIndex >= 0 && SlotIndex < MaxSlots)
    {
        CurrentSelectedSlot = SlotIndex;
        UE_LOG(LogTemp, Warning, TEXT("Selected slot %d"), SlotIndex);
        
        // 해당 슬롯에 아이템이 있으면 자동 장착
        ACYItemBase* Item = GetItemAtSlot(SlotIndex);
        if (Item)
        {
            AutoEquipItem(Item);
        }
        
        OnInventoryUpdated.Broadcast();
    }
}

ACYItemBase* UCYInventoryComponent::GetSelectedItem()
{
    return GetItemAtSlot(CurrentSelectedSlot);
}

void UCYInventoryComponent::UseSelectedItem()
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
    if (!Player) return;

    // 장착된 무기가 있으면 공격
    if (Player->WeaponManager && Player->WeaponManager->CurrentWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attacking with equipped weapon"));
        FHitResult HitResult;
        if (Player->PerformLineTrace(HitResult))
        {
            Player->WeaponManager->ServerUseWeapon(HitResult.Location, HitResult.GetActor());
        }
        return;
    }

    // 장착된 무기가 없으면 선택된 아이템 사용
    ACYItemBase* SelectedItem = GetSelectedItem();
    if (!SelectedItem) 
    {
        UE_LOG(LogTemp, Warning, TEXT("No item selected and no weapon equipped"));
        return;
    }

    // 트랩인지 확인
    if (ACYTrapBase* TrapItem = Cast<ACYTrapBase>(SelectedItem))
    {
        UE_LOG(LogTemp, Warning, TEXT("Placing trap"));
        FVector PlaceLocation = Player->GetActorLocation() + Player->GetActorForwardVector() * 200.0f;
        Player->TrapManager->ServerPlaceTrap(TrapItem->GetClass(), PlaceLocation, Player->GetActorRotation());
        ServerRemoveItem(CurrentSelectedSlot);
    }
    // 소모품
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Using consumable"));
        SelectedItem->OnItemUsed(GetOwner());
        ServerRemoveItem(CurrentSelectedSlot);
    }
}

void UCYInventoryComponent::AutoEquipItem(ACYItemBase* Item)
{
    if (!Item) return;
    
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
    if (!Player) return;

    UE_LOG(LogTemp, Warning, TEXT("Auto-equipping item: %s"), *Item->GetName());
    
    // 모든 아이템을 장착 (무기, 트랩, 소모품 모두)
    Player->ServerEquipItem(Item);
}

void UCYInventoryComponent::ServerRemoveItemByReference_Implementation(ACYItemBase* Item)
{
    if (!GetOwner()->HasAuthority() || !Item) return;

    for (int32 i = 0; i < Items.Num(); i++)
    {
        if (Items[i] == Item)
        {
            Items[i] = nullptr;
            OnInventoryUpdated.Broadcast();
            UE_LOG(LogTemp, Warning, TEXT("Removed item %s from slot %d"), *Item->GetName(), i);
            break;
        }
    }
}