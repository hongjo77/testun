// CYInventoryComponent.cpp - 중복 실행 방지 및 단순화
#include "Components/CYInventoryComponent.h"
#include "Items/CYItemBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CYGameplayTags.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYWeaponBase.h"
#include "CYInventoryTypes.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCYInventoryComponent::UCYInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    
    // ✅ 중복 사용 방지
    bIsProcessingUse = false;
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
    DOREPLIFETIME(UCYInventoryComponent, bIsProcessingUse); // ✅ 처리 상태 동기화
}

bool UCYInventoryComponent::AddItem(ACYItemBase* Item, int32 SlotIndex)
{
    if (!Item) return false;

    FGameplayTag WeaponTag = FGameplayTag::RequestGameplayTag("Item.Weapon");
    bool bIsWeapon = Item->ItemTag.MatchesTag(WeaponTag);
    
    if (bIsWeapon)
    {
        return AddWeapon(Item);
    }
    else
    {
        return AddItemWithStacking(Item);
    }
}

bool UCYInventoryComponent::RemoveItem(int32 SlotIndex)
{
    EInventorySlotType SlotType;
    int32 LocalIndex;
    
    if (!UInventorySlotUtils::ParseSlotIndex(SlotIndex, SlotType, LocalIndex))
    {
        return false;
    }
    
    switch (SlotType)
    {
        case EInventorySlotType::Weapon:
            return RemoveWeaponFromSlot(LocalIndex);
            
        case EInventorySlotType::Item:
            return RemoveItemFromSlot(LocalIndex);
            
        default:
            return false;
    }
}

ACYItemBase* UCYInventoryComponent::GetItem(int32 SlotIndex) const
{
    EInventorySlotType SlotType;
    int32 LocalIndex;
    
    if (!UInventorySlotUtils::ParseSlotIndex(SlotIndex, SlotType, LocalIndex))
    {
        return nullptr;
    }
    
    switch (SlotType)
    {
        case EInventorySlotType::Weapon:
            if (LocalIndex >= 0 && LocalIndex < WeaponSlots.Num())
            {
                return WeaponSlots[LocalIndex];
            }
            break;
            
        case EInventorySlotType::Item:
            if (LocalIndex >= 0 && LocalIndex < ItemSlots.Num())
            {
                return ItemSlots[LocalIndex];
            }
            break;
    }
    
    return nullptr;
}

bool UCYInventoryComponent::UseItem(int32 SlotIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("📦 UCYInventoryComponent::UseItem called with SlotIndex: %d"), SlotIndex);

    // ✅ 중복 사용 방지
    if (bIsProcessingUse)
    {
        UE_LOG(LogTemp, Warning, TEXT("📦 Already processing item use, ignoring"));
        return false;
    }

    if (!GetOwner()->HasAuthority()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("📦 Not authority, calling ServerUseItem"));
        ServerUseItem(SlotIndex);
        return false;
    }

    // ✅ 처리 시작 플래그 설정
    bIsProcessingUse = true;

    ACYItemBase* Item = GetItem(SlotIndex);
    UE_LOG(LogTemp, Warning, TEXT("📦 GetItem result: %s"), Item ? *Item->ItemName.ToString() : TEXT("NULL"));
    
    if (!Item) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No item found at SlotIndex: %d"), SlotIndex);
        bIsProcessingUse = false;
        return false;
    }

    EInventorySlotType SlotType;
    int32 LocalIndex;
    UInventorySlotUtils::ParseSlotIndex(SlotIndex, SlotType, LocalIndex);
    
    UE_LOG(LogTemp, Warning, TEXT("📦 SlotType: %s, LocalIndex: %d"), 
           SlotType == EInventorySlotType::Weapon ? TEXT("Weapon") : TEXT("Item"), LocalIndex);

    bool bResult = false;

    // 무기 장착
    if (SlotType == EInventorySlotType::Weapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("📦 Trying to equip weapon"));
        bResult = EquipWeaponFromSlot(Item);
    }
    else
    {
        // ✅ 트랩 아이템 사용 - 직접 GA_PlaceTrap 실행
        FGameplayTag TrapTag = FGameplayTag::RequestGameplayTag("Item.Trap");
        if (Item->ItemTag.MatchesTag(TrapTag))
        {
            UE_LOG(LogTemp, Warning, TEXT("📦 Trying to use trap item: %s"), *Item->ItemName.ToString());
            bResult = UseTrapItemDirect(Item, LocalIndex);
        }
        else
        {
            // 일반 아이템 사용
            UE_LOG(LogTemp, Warning, TEXT("📦 Trying to activate item ability for: %s"), *Item->ItemName.ToString());
            bResult = ActivateItemAbility(Item, LocalIndex);
        }
    }

    // ✅ 처리 완료 후 플래그 해제
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        bIsProcessingUse = false;
    });

    return bResult;
}

bool UCYInventoryComponent::UseTrapItemDirect(ACYItemBase* Item, int32 LocalIndex)
{
    if (!Item) return false;

    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No AbilitySystemComponent found"));
        return false;
    }

    UCYAbilitySystemComponent* CyASC = Cast<UCYAbilitySystemComponent>(ASC);
    if (!CyASC)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ASC is not UCYAbilitySystemComponent"));
        return false;
    }

    // ✅ 트랩 배치 어빌리티 한 번만 실행
    FGameplayTag TrapPlaceTag = FGameplayTag::RequestGameplayTag("Ability.Trap.Place");
    
    // ✅ 쿨다운 체크
    FGameplayTagContainer CooldownTags;
    CooldownTags.AddTag(FGameplayTag::RequestGameplayTag("Cooldown.Trap.Place"));
    if (CyASC->HasAnyMatchingGameplayTags(CooldownTags))
    {
        UE_LOG(LogTemp, Warning, TEXT("⏰ Trap placement on cooldown"));
        return false;
    }

    bool bSuccess = CyASC->TryActivateAbilityByTag(TrapPlaceTag);
    
    UE_LOG(LogTemp, Warning, TEXT("🎯 TrapPlace ability result: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚡ Trap placement initiated successfully"));
    }
    
    return bSuccess;
}

void UCYInventoryComponent::ServerUseItem_Implementation(int32 SlotIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("🌐 ServerUseItem called with SlotIndex: %d"), SlotIndex);
    bool bResult = UseItem(SlotIndex);
    UE_LOG(LogTemp, Warning, TEXT("🌐 ServerUseItem result: %s"), bResult ? TEXT("SUCCESS") : TEXT("FAILED"));
}

// ============ 기존 핵심 로직 (유지) ============

bool UCYInventoryComponent::AddWeapon(ACYItemBase* Weapon)
{
    if (!Weapon) return false;

    int32 EmptySlot = FindEmptyWeaponSlot();
    if (EmptySlot == -1) return false;

    WeaponSlots[EmptySlot] = Weapon;
    
    int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Weapon, EmptySlot);
    OnInventoryChanged.Broadcast(UnifiedIndex, Weapon);
    
    AutoEquipFirstWeapon(Cast<ACYWeaponBase>(Weapon));
    
    return true;
}

bool UCYInventoryComponent::AddItemWithStacking(ACYItemBase* Item)
{
    if (!Item) return false;

    if (TryStackWithExistingItem(Item))
    {
        return true;
    }

    int32 EmptySlot = FindEmptyItemSlot();
    if (EmptySlot == -1) return false;

    ItemSlots[EmptySlot] = Item;
    
    int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Item, EmptySlot);
    OnInventoryChanged.Broadcast(UnifiedIndex, Item);
    
    return true;
}

// === 헬퍼 함수들 (기존 유지) ===

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
    
    int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Item, StackableSlot);
    OnInventoryChanged.Broadcast(UnifiedIndex, ExistingItem);
    
    if (Item->ItemCount <= 0)
    {
        Item->Destroy();
        return true;
    }
    
    return false;
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
    UE_LOG(LogTemp, Warning, TEXT("⚡ ActivateItemAbility called for item: %s"), 
           Item ? *Item->ItemName.ToString() : TEXT("NULL"));

    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No AbilitySystemComponent found"));
        return false;
    }

    UCYAbilitySystemComponent* CyASC = Cast<UCYAbilitySystemComponent>(ASC);
    if (!CyASC)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ASC is not UCYAbilitySystemComponent"));
        return false;
    }

    if (!Item->ItemAbility) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Item has no ItemAbility: %s"), *Item->ItemName.ToString());
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("⚡ Looking for ability: %s"), *Item->ItemAbility->GetName());

    FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag("Event.Item.Use");
    bool bSuccess = CyASC->TryActivateAbilityByTag(AbilityTag);
    
    UE_LOG(LogTemp, Warning, TEXT("⚡ Item ability result: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚡ Item ability activated successfully"));
        
        FGameplayTag ConsumableTag = FGameplayTag::RequestGameplayTag("Item.Consumable");
        if (Item->ItemTag.MatchesTag(ConsumableTag))
        {
            ProcessItemConsumption(Item, SlotIndex);
        }
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
            
            int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Item, SlotIndex);
            OnInventoryChanged.Broadcast(UnifiedIndex, nullptr);
            Item->Destroy();
        }
        else
        {
            int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Item, SlotIndex);
            OnInventoryChanged.Broadcast(UnifiedIndex, Item);
        }
    }
}

bool UCYInventoryComponent::RemoveWeaponFromSlot(int32 WeaponIndex)
{
    if (WeaponIndex >= 0 && WeaponIndex < WeaponSlots.Num() && WeaponSlots[WeaponIndex])
    {
        WeaponSlots[WeaponIndex] = nullptr;
        
        int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Weapon, WeaponIndex);
        OnInventoryChanged.Broadcast(UnifiedIndex, nullptr);
        return true;
    }
    return false;
}

bool UCYInventoryComponent::RemoveItemFromSlot(int32 ItemIndex)
{
    if (ItemIndex >= 0 && ItemIndex < ItemSlots.Num() && ItemSlots[ItemIndex])
    {
        ItemSlots[ItemIndex] = nullptr;
        
        int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Item, ItemIndex);
        OnInventoryChanged.Broadcast(UnifiedIndex, nullptr);
        return true;
    }
    return false;
}

void UCYInventoryComponent::OnRep_WeaponSlots()
{
    for (int32 i = 0; i < WeaponSlots.Num(); ++i)
    {
        int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Weapon, i);
        OnInventoryChanged.Broadcast(UnifiedIndex, WeaponSlots[i]);
    }
}

void UCYInventoryComponent::OnRep_ItemSlots()
{
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Item, i);
        OnInventoryChanged.Broadcast(UnifiedIndex, ItemSlots[i]);
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