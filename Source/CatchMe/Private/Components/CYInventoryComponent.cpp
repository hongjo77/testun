#include "Components/CYInventoryComponent.h"
#include "Items/CYItemBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CYGameplayTags.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYWeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCYInventoryComponent::UCYInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    WeaponSlots.SetNum(WeaponSlotCount);
    ItemSlots.SetNum(ItemSlotCount);
}

void UCYInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCYInventoryComponent, WeaponSlots);
    DOREPLIFETIME(UCYInventoryComponent, ItemSlots);
}

bool UCYInventoryComponent::AddItem(ACYItemBase* Item, int32 SlotIndex)
{
    if (!Item) return false;

    FGameplayTag WeaponTag = FGameplayTag::RequestGameplayTag("Item.Weapon");
    
    if (Item->ItemTag.MatchesTag(WeaponTag))
    {
        return AddWeapon(Item);
    }
    else
    {
        return AddItemWithStacking(Item);
    }
}

bool UCYInventoryComponent::AddWeapon(ACYItemBase* Weapon)
{
    if (!Weapon) return false;

    int32 EmptySlot = FindEmptyWeaponSlot();
    if (EmptySlot == -1) return false;

    WeaponSlots[EmptySlot] = Weapon;
    OnInventoryChanged.Broadcast(EmptySlot + 1000, Weapon);
    
    // 첫 번째 무기 자동 장착
    AutoEquipFirstWeapon(Cast<ACYWeaponBase>(Weapon));
    
    UE_LOG(LogTemp, Log, TEXT("Weapon added to slot %d"), EmptySlot);
    return true;
}

bool UCYInventoryComponent::AddItemWithStacking(ACYItemBase* Item)
{
    if (!Item) return false;

    // 1. 기존 스택에 추가 시도
    if (TryStackWithExistingItem(Item))
    {
        return true;
    }

    // 2. 새 슬롯에 추가
    int32 EmptySlot = FindEmptyItemSlot();
    if (EmptySlot == -1) return false;

    ItemSlots[EmptySlot] = Item;
    OnInventoryChanged.Broadcast(EmptySlot, Item);
    
    UE_LOG(LogTemp, Log, TEXT("Item added to slot %d"), EmptySlot);
    return true;
}

bool UCYInventoryComponent::UseItem(int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority()) return false;

    ACYItemBase* Item = GetItem(SlotIndex);
    if (!Item) return false;

    // 무기 장착
    if (SlotIndex >= 1000)
    {
        return EquipWeaponFromSlot(Item);
    }

    // 일반 아이템 사용
    return ActivateItemAbility(Item, SlotIndex);
}

bool UCYInventoryComponent::RemoveItem(int32 SlotIndex)
{
    if (SlotIndex >= 1000) // 무기 슬롯
    {
        return RemoveWeaponFromSlot(SlotIndex - 1000);
    }
    else // 아이템 슬롯
    {
        return RemoveItemFromSlot(SlotIndex);
    }
}

ACYItemBase* UCYInventoryComponent::GetItem(int32 SlotIndex) const
{
    if (SlotIndex >= 1000) // 무기 슬롯
    {
        int32 WeaponIndex = SlotIndex - 1000;
        return (WeaponIndex >= 0 && WeaponIndex < WeaponSlots.Num()) ? WeaponSlots[WeaponIndex] : nullptr;
    }
    else // 아이템 슬롯
    {
        return (SlotIndex >= 0 && SlotIndex < ItemSlots.Num()) ? ItemSlots[SlotIndex] : nullptr;
    }
}

void UCYInventoryComponent::ServerUseItem_Implementation(int32 SlotIndex)
{
    UseItem(SlotIndex);
}

// === 헬퍼 함수들 ===

int32 UCYInventoryComponent::FindEmptyWeaponSlot() const
{
    for (int32 i = 0; i < WeaponSlots.Num(); ++i)
    {
        if (!WeaponSlots[i]) return i;
    }
    return -1;
}

int32 UCYInventoryComponent::FindEmptyItemSlot() const
{
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (!ItemSlots[i]) return i;
    }
    return -1;
}

int32 UCYInventoryComponent::FindStackableItemSlot(ACYItemBase* Item) const
{
    if (!Item) return -1;
    
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (ItemSlots[i] && ItemSlots[i]->CanStackWith(Item))
        {
            return i;
        }
    }
    return -1;
}

bool UCYInventoryComponent::TryStackWithExistingItem(ACYItemBase* Item)
{
    int32 StackableSlot = FindStackableItemSlot(Item);
    if (StackableSlot == -1) return false;

    ACYItemBase* ExistingItem = ItemSlots[StackableSlot];
    int32 AddableCount = FMath::Min(Item->ItemCount, 
                                    ExistingItem->MaxStackCount - ExistingItem->ItemCount);
    
    ExistingItem->ItemCount += AddableCount;
    Item->ItemCount -= AddableCount;
    
    OnInventoryChanged.Broadcast(StackableSlot, ExistingItem);
    
    // 아이템이 모두 스택되면 원본 제거
    if (Item->ItemCount <= 0)
    {
        Item->Destroy();
        return true;
    }
    
    return false; // 일부만 스택됨 - 새 슬롯 필요
}

void UCYInventoryComponent::AutoEquipFirstWeapon(ACYWeaponBase* Weapon)
{
    if (!Weapon) return;

    UCYWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UCYWeaponComponent>();
    if (WeaponComp && !WeaponComp->CurrentWeapon)
    {
        WeaponComp->EquipWeapon(Weapon);
    }
}

bool UCYInventoryComponent::EquipWeaponFromSlot(ACYItemBase* Item)
{
    if (ACYWeaponBase* Weapon = Cast<ACYWeaponBase>(Item))
    {
        UCYWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UCYWeaponComponent>();
        return WeaponComp ? WeaponComp->EquipWeapon(Weapon) : false;
    }
    return false;
}

bool UCYInventoryComponent::ActivateItemAbility(ACYItemBase* Item, int32 SlotIndex)
{
    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC || !Item->ItemAbility) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ASC or ItemAbility is null"));
        return false;
    }

    FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(Item->ItemAbility);
    if (!Spec) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ AbilitySpec not found for %s"), *Item->ItemAbility->GetName());
        return false;
    }

    // ✅ 완전히 단순한 방식: SourceObject만 설정
    Spec->SourceObject = Item;
    
    UE_LOG(LogTemp, Warning, TEXT("🎯 Trying to activate ability for item: %s"), *Item->ItemName.ToString());
    
    bool bSuccess = ASC->TryActivateAbility(Spec->Handle);
    
    UE_LOG(LogTemp, Warning, TEXT("🎯 Item ability activation result: %s, Success=%s"), 
           *Item->ItemName.ToString(), bSuccess ? TEXT("true") : TEXT("false"));
    
    if (bSuccess)
    {
        ProcessItemConsumption(Item, SlotIndex);
    }
    
    return bSuccess;
}

void UCYInventoryComponent::ProcessItemConsumption(ACYItemBase* Item, int32 SlotIndex)
{
    FGameplayTag ConsumableTag = FGameplayTag::RequestGameplayTag("Item.Consumable");
    FGameplayTag TrapTag = FGameplayTag::RequestGameplayTag("Item.Trap");
    
    if (Item->ItemTag.MatchesTag(ConsumableTag) || Item->ItemTag.MatchesTag(TrapTag))
    {
        Item->ItemCount--;
        if (Item->ItemCount <= 0)
        {
            ItemSlots[SlotIndex] = nullptr;
            OnInventoryChanged.Broadcast(SlotIndex, nullptr);
            Item->Destroy();
        }
        else
        {
            OnInventoryChanged.Broadcast(SlotIndex, Item);
        }
    }
}

bool UCYInventoryComponent::RemoveWeaponFromSlot(int32 WeaponIndex)
{
    if (WeaponIndex >= 0 && WeaponIndex < WeaponSlots.Num() && WeaponSlots[WeaponIndex])
    {
        WeaponSlots[WeaponIndex] = nullptr;
        OnInventoryChanged.Broadcast(WeaponIndex + 1000, nullptr);
        return true;
    }
    return false;
}

bool UCYInventoryComponent::RemoveItemFromSlot(int32 ItemIndex)
{
    if (ItemIndex >= 0 && ItemIndex < ItemSlots.Num() && ItemSlots[ItemIndex])
    {
        ItemSlots[ItemIndex] = nullptr;
        OnInventoryChanged.Broadcast(ItemIndex, nullptr);
        return true;
    }
    return false;
}

void UCYInventoryComponent::OnRep_WeaponSlots()
{
    for (int32 i = 0; i < WeaponSlots.Num(); ++i)
    {
        OnInventoryChanged.Broadcast(i + 1000, WeaponSlots[i]);
    }
}

void UCYInventoryComponent::OnRep_ItemSlots()
{
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        OnInventoryChanged.Broadcast(i, ItemSlots[i]);
    }
}

UAbilitySystemComponent* UCYInventoryComponent::GetOwnerASC() const
{
    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
    {
        return ASI->GetAbilitySystemComponent();
    }
    return nullptr;
}