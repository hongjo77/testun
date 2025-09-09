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
        
        // UI 업데이트 이벤트
        OnInventoryUpdated.Broadcast();
    }
}

ACYItemBase* UCYInventoryComponent::GetSelectedItem()
{
    return GetItemAtSlot(CurrentSelectedSlot);
}

void UCYInventoryComponent::UseSelectedItem()
{
    ACYItemBase* SelectedItem = GetSelectedItem();
    if (!SelectedItem) 
    {
        UE_LOG(LogTemp, Warning, TEXT("No item selected"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Using selected item: %s"), *SelectedItem->GetName());

    // 무기인지 확인
    if (ACYWeaponBase* WeaponItem = Cast<ACYWeaponBase>(SelectedItem))
    {
        // 무기 장착/공격 로직
        APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
        if (Player && Player->WeaponManager)
        {
            if (Player->WeaponManager->CurrentWeapon == WeaponItem)
            {
                // 이미 장착된 무기면 공격
                UE_LOG(LogTemp, Warning, TEXT("Attacking with weapon"));
                FHitResult HitResult;
                if (Player->PerformLineTrace(HitResult))
                {
                    Player->WeaponManager->ServerUseWeapon(HitResult.Location, HitResult.GetActor());
                }
            }
            else
            {
                // 다른 무기면 장착
                UE_LOG(LogTemp, Warning, TEXT("Equipping weapon"));
                Player->WeaponManager->ServerEquipWeapon(WeaponItem);
                ServerRemoveItem(CurrentSelectedSlot);
            }
        }
    }
    // 트랩인지 확인
    else if (ACYTrapBase* TrapItem = Cast<ACYTrapBase>(SelectedItem))
    {
        UE_LOG(LogTemp, Warning, TEXT("Placing trap"));
        APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
        if (Player && Player->TrapManager)
        {
            FVector PlaceLocation = Player->GetActorLocation() + Player->GetActorForwardVector() * 200.0f;
            Player->TrapManager->ServerPlaceTrap(TrapItem->GetClass(), PlaceLocation, Player->GetActorRotation());
            ServerRemoveItem(CurrentSelectedSlot);
        }
    }
    // 소모품
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Using consumable"));
        SelectedItem->OnItemUsed(GetOwner());
        ServerRemoveItem(CurrentSelectedSlot);
    }
}