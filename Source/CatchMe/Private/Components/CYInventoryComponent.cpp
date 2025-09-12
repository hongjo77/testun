#include "Components/CYInventoryComponent.h"
#include "Items/CYItemBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CYGameplayTags.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYWeaponBase.h"
#include "Net/UnrealNetwork.h"

UCYInventoryComponent::UCYInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // ✅ 분리된 슬롯 크기 설정
    WeaponSlots.SetNum(WeaponSlotCount);
    ItemSlots.SetNum(ItemSlotCount);
}

void UCYInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCYInventoryComponent, WeaponSlots);
    DOREPLIFETIME(UCYInventoryComponent, ItemSlots);
}

int32 UCYInventoryComponent::FindEmptySlot() const
{
    // 아이템 슬롯에서 빈 공간 찾기
    return FindEmptyItemSlot();
}

int32 UCYInventoryComponent::FindEmptyWeaponSlot() const
{
    for (int32 i = 0; i < WeaponSlots.Num(); ++i)
    {
        if (!WeaponSlots[i])
        {
            return i;
        }
    }
    return -1;
}

int32 UCYInventoryComponent::FindEmptyItemSlot() const
{
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (!ItemSlots[i])
        {
            return i;
        }
    }
    return -1;
}

int32 UCYInventoryComponent::FindStackableItemSlot(ACYItemBase* Item) const
{
    if (!Item) 
    {
        UE_LOG(LogTemp, Warning, TEXT("FindStackableItemSlot: Item is null"));
        return -1;
    }
    
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (ItemSlots[i] && ItemSlots[i]->CanStackWith(Item))
        {
            UE_LOG(LogTemp, Warning, TEXT("FindStackableItemSlot: Found stackable slot %d"), i);
            return i;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("FindStackableItemSlot: No stackable slot found"));
    return -1;
}

bool UCYInventoryComponent::AddItemWithStacking(ACYItemBase* Item)
{
    if (!Item) return false;

    UE_LOG(LogTemp, Warning, TEXT("AddItemWithStacking: Adding %s (count: %d)"), 
           *Item->ItemName.ToString(), Item->ItemCount);

    // 1. 스택 가능한 슬롯 찾기
    int32 StackableSlot = FindStackableItemSlot(Item);
    if (StackableSlot != -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddItemWithStacking: Found stackable slot %d"), StackableSlot);
        
        // 스택에 추가
        ACYItemBase* ExistingItem = ItemSlots[StackableSlot];
        int32 AddableCount = FMath::Min(Item->ItemCount, 
                                      ExistingItem->MaxStackCount - ExistingItem->ItemCount);
        
        UE_LOG(LogTemp, Warning, TEXT("AddItemWithStacking: Adding %d to existing stack of %d"), 
               AddableCount, ExistingItem->ItemCount);
        
        ExistingItem->ItemCount += AddableCount;
        Item->ItemCount -= AddableCount;
        
        OnInventoryChanged.Broadcast(StackableSlot, ExistingItem);
        
        // 아이템이 모두 스택되었으면 원본 아이템 제거
        if (Item->ItemCount <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("AddItemWithStacking: All items stacked, destroying original"));
            Item->Destroy();
            return true;
        }
    }

    // 2. 새 슬롯에 추가 (남은 수량이 있다면)
    int32 EmptySlot = FindEmptyItemSlot();
    if (EmptySlot != -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddItemWithStacking: Adding to empty slot %d (key %d)"), 
               EmptySlot, EmptySlot + 4);
        
        ItemSlots[EmptySlot] = Item;
        OnInventoryChanged.Broadcast(EmptySlot, Item);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("AddItemWithStacking: No empty item slots"));
    return false; // 인벤토리 가득참
}

bool UCYInventoryComponent::AddWeapon(ACYItemBase* Weapon)
{
    if (!Weapon) return false;

    UE_LOG(LogTemp, Warning, TEXT("AddWeapon: Adding %s"), *Weapon->ItemName.ToString());

    int32 EmptySlot = FindEmptyWeaponSlot();
    if (EmptySlot != -1)
    {
        WeaponSlots[EmptySlot] = Weapon;
        OnInventoryChanged.Broadcast(EmptySlot + 1000, Weapon); // 무기는 1000번대로 구분
        
        UE_LOG(LogTemp, Warning, TEXT("AddWeapon: Added to weapon slot %d (key %d)"), EmptySlot, EmptySlot + 1);
        
        // ✅ 첫 번째 무기 자동 장착
        UCYWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UCYWeaponComponent>();
        if (WeaponComp && !WeaponComp->CurrentWeapon)
        {
            if (ACYWeaponBase* WeaponBase = Cast<ACYWeaponBase>(Weapon))
            {
                bool bEquipped = WeaponComp->EquipWeapon(WeaponBase);
                UE_LOG(LogTemp, Warning, TEXT("AddWeapon: Auto-equipped result: %s"), 
                       bEquipped ? TEXT("SUCCESS") : TEXT("FAILED"));
            }
        }
        
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("AddWeapon: No empty weapon slots"));
    return false; // 무기 슬롯 가득참
}

bool UCYInventoryComponent::AddItem(ACYItemBase* Item, int32 SlotIndex)
{
    if (!Item) return false;

    UE_LOG(LogTemp, Warning, TEXT("UCYInventoryComponent::AddItem: %s with tag %s"), 
           *Item->ItemName.ToString(), *Item->ItemTag.ToString());

    // ✅ 하드코딩 태그로 확인
    FGameplayTag WeaponTag = FGameplayTag::RequestGameplayTag("Item.Weapon");
    
    UE_LOG(LogTemp, Warning, TEXT("Checking weapon tag: Item=%s vs Weapon=%s"), 
           *Item->ItemTag.ToString(), *WeaponTag.ToString());
    
    if (Item->ItemTag.MatchesTag(WeaponTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("Item is weapon, adding to weapon slots"));
        return AddWeapon(Item);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Item is not weapon, adding to item slots"));
        return AddItemWithStacking(Item);
    }
}

bool UCYInventoryComponent::RemoveItem(int32 SlotIndex)
{
    if (SlotIndex >= 1000) // 무기 슬롯
    {
        int32 WeaponIndex = SlotIndex - 1000;
        if (WeaponIndex >= 0 && WeaponIndex < WeaponSlots.Num() && WeaponSlots[WeaponIndex])
        {
            WeaponSlots[WeaponIndex] = nullptr;
            OnInventoryChanged.Broadcast(SlotIndex, nullptr);
            return true;
        }
    }
    else // 아이템 슬롯
    {
        if (SlotIndex >= 0 && SlotIndex < ItemSlots.Num() && ItemSlots[SlotIndex])
        {
            ItemSlots[SlotIndex] = nullptr;
            OnInventoryChanged.Broadcast(SlotIndex, nullptr);
            return true;
        }
    }
    return false;
}

ACYItemBase* UCYInventoryComponent::GetItem(int32 SlotIndex) const
{
    if (SlotIndex >= 1000) // 무기 슬롯
    {
        int32 WeaponIndex = SlotIndex - 1000;
        if (WeaponIndex >= 0 && WeaponIndex < WeaponSlots.Num())
        {
            return WeaponSlots[WeaponIndex];
        }
    }
    else // 아이템 슬롯
    {
        if (SlotIndex >= 0 && SlotIndex < ItemSlots.Num())
        {
            return ItemSlots[SlotIndex];
        }
    }
    return nullptr;
}

// CYInventoryComponent.cpp - UseItem 함수 전체 (기존 파일에서 이 함수만 교체)
bool UCYInventoryComponent::UseItem(int32 SlotIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("🎯 === USE ITEM DEBUGGING ==="));
    UE_LOG(LogTemp, Warning, TEXT("UCYInventoryComponent::UseItem called with SlotIndex: %d"), SlotIndex);
    
    if (!GetOwner()->HasAuthority()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: No authority"));
        return false;
    }

    ACYItemBase* Item = GetItem(SlotIndex);
    if (!Item) 
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: No item in slot %d"), SlotIndex);
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("UseItem: Found item %s in slot %d"), *Item->ItemName.ToString(), SlotIndex);
    UE_LOG(LogTemp, Warning, TEXT("Item class: %s"), *Item->GetClass()->GetName());

    // 🔍 트랩 아이템인 경우 DesiredTrapEffects 확인
    FGameplayTag TrapTag = FGameplayTag::RequestGameplayTag("Item.Trap");
    if (Item->ItemTag.MatchesTag(TrapTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("🎯 This is a TRAP item!"));
        UE_LOG(LogTemp, Warning, TEXT("DesiredTrapEffects.Num(): %d"), Item->DesiredTrapEffects.Num());
        
        for (int32 i = 0; i < Item->DesiredTrapEffects.Num(); i++)
        {
            if (Item->DesiredTrapEffects[i])
            {
                UE_LOG(LogTemp, Warning, TEXT("  DesiredEffect[%d]: %s"), 
                       i, *Item->DesiredTrapEffects[i]->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("  DesiredEffect[%d]: NULL"), i);
            }
        }
    }

    // 무기 슬롯(1000번대)인 경우 WeaponComponent에 장착
    if (SlotIndex >= 1000)
    {
        if (ACYWeaponBase* Weapon = Cast<ACYWeaponBase>(Item))
        {
            UE_LOG(LogTemp, Warning, TEXT("UseItem: Weapon slot clicked, equipping weapon"));
            UCYWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UCYWeaponComponent>();
            if (WeaponComp)
            {
                return WeaponComp->EquipWeapon(Weapon);
            }
        }
        return false;
    }

    // 일반 아이템 처리
    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC) 
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: No ASC found"));
        return false;
    }

    if (!Item->ItemAbility) 
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Item %s has no ability"), *Item->ItemName.ToString());
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("UseItem: Item %s has ability %s"), 
           *Item->ItemName.ToString(), *Item->ItemAbility->GetName());

    FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(Item->ItemAbility);
    if (Spec)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Found ability spec for %s"), *Item->ItemAbility->GetName());
        
        // 🔍 SourceObject 설정 디버깅
        UE_LOG(LogTemp, Warning, TEXT("🎯 SETTING SourceObject to: %s"), *Item->GetName());
        UE_LOG(LogTemp, Warning, TEXT("Item address: %p"), Item);
        
        Spec->SourceObject = Item;
        
        // 설정 확인
        if (Spec->SourceObject.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ SourceObject set successfully: %s"), 
                   *Spec->SourceObject->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ SourceObject setting FAILED!"));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("🚀 Trying to activate ability..."));
        bool bSuccess = ASC->TryActivateAbility(Spec->Handle);
        UE_LOG(LogTemp, Warning, TEXT("Ability activation result: %s"), 
               bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
        
        if (bSuccess)
        {
            // 아이템 사용 후 처리
            FGameplayTag ConsumableTag = FGameplayTag::RequestGameplayTag("Item.Consumable");
            
            if (Item->ItemTag.MatchesTag(ConsumableTag) || Item->ItemTag.MatchesTag(TrapTag))
            {
                UE_LOG(LogTemp, Warning, TEXT("UseItem: Item is consumable/trap, reducing count"));
                Item->ItemCount--;
                if (Item->ItemCount <= 0)
                {
                    UE_LOG(LogTemp, Warning, TEXT("UseItem: Item count reached 0, removing from slot"));
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
        
        UE_LOG(LogTemp, Warning, TEXT("🎯 === END USE ITEM DEBUGGING ==="));
        return bSuccess;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Could not find ability spec for item %s"), *Item->ItemName.ToString());
    }
    
    return false;
}

void UCYInventoryComponent::ServerUseItem_Implementation(int32 SlotIndex)
{
    UseItem(SlotIndex);
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

void UCYInventoryComponent::PrintInventoryStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== 인벤토리 상태 ==="));
    
    // 무기 슬롯 (1~3번 키)
    UE_LOG(LogTemp, Warning, TEXT("🗡️ 무기 슬롯 (1~3번 키):"));
    for (int32 i = 0; i < WeaponSlots.Num(); ++i)
    {
        if (WeaponSlots[i])
        {
            UE_LOG(LogTemp, Warning, TEXT("  [%d번 키] %s x%d"), 
                   i + 1, *WeaponSlots[i]->ItemName.ToString(), WeaponSlots[i]->ItemCount);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  [%d번 키] 비어있음"), i + 1);
        }
    }
    
    // 아이템 슬롯 (4~9번 키)
    UE_LOG(LogTemp, Warning, TEXT("🎒 아이템 슬롯 (4~9번 키):"));
    for (int32 i = 0; i < ItemSlots.Num() && i < 6; ++i) // 6개만 표시 (4~9번 키)
    {
        if (ItemSlots[i])
        {
            UE_LOG(LogTemp, Warning, TEXT("  [%d번 키] %s x%d"), 
                   i + 4, *ItemSlots[i]->ItemName.ToString(), ItemSlots[i]->ItemCount);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  [%d번 키] 비어있음"), i + 4);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=================="));
}

UAbilitySystemComponent* UCYInventoryComponent::GetOwnerASC() const
{
    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
    {
        return ASI->GetAbilitySystemComponent();
    }
    return nullptr;
}