#include "Components/CYInventoryComponent.h"
#include "Items/CYItemBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CYGameplayTags.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYWeaponBase.h"
#include "CYInventoryTypes.h" // ✅ 새로운 타입 시스템
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
    // ✅ 개선된 시스템: 타입 안전한 슬롯 파싱
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
    // ✅ 개선된 시스템: 타입 안전한 슬롯 파싱
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

    if (!GetOwner()->HasAuthority()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("📦 Not authority, calling ServerUseItem"));
        ServerUseItem(SlotIndex);
        return false; // ✅ 여기서 리턴해서 중복 실행 방지
    }

    ACYItemBase* Item = GetItem(SlotIndex);
    UE_LOG(LogTemp, Warning, TEXT("📦 GetItem result: %s"), Item ? *Item->ItemName.ToString() : TEXT("NULL"));
    
    if (!Item) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No item found at SlotIndex: %d"), SlotIndex);
        return false;
    }

    // ✅ 개선된 시스템으로 슬롯 타입 확인
    EInventorySlotType SlotType;
    int32 LocalIndex;
    UInventorySlotUtils::ParseSlotIndex(SlotIndex, SlotType, LocalIndex);
    
    UE_LOG(LogTemp, Warning, TEXT("📦 SlotType: %s, LocalIndex: %d"), 
           SlotType == EInventorySlotType::Weapon ? TEXT("Weapon") : TEXT("Item"), LocalIndex);

    // 무기 장착
    if (SlotType == EInventorySlotType::Weapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("📦 Trying to equip weapon"));
        return EquipWeaponFromSlot(Item);
    }

    // 일반 아이템 사용
    UE_LOG(LogTemp, Warning, TEXT("📦 Trying to activate item ability for: %s"), *Item->ItemName.ToString());
    return ActivateItemAbility(Item, LocalIndex);
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
    
    // ✅ 개선된 시스템으로 이벤트 발생
    int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Weapon, EmptySlot);
    OnInventoryChanged.Broadcast(UnifiedIndex, Weapon);
    
    // 첫 번째 무기 자동 장착
    AutoEquipFirstWeapon(Cast<ACYWeaponBase>(Weapon));
    
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
    
    // ✅ 개선된 시스템으로 이벤트 발생
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
    
    // ✅ 개선된 시스템으로 이벤트 발생
    int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Item, StackableSlot);
    OnInventoryChanged.Broadcast(UnifiedIndex, ExistingItem);
    
    // 아이템이 모두 스택되면 원본 제거
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

    if (!Item->ItemAbility) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Item has no ItemAbility: %s"), *Item->ItemName.ToString());
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("⚡ Looking for ability: %s"), *Item->ItemAbility->GetName());

    FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(Item->ItemAbility);
    if (!Spec) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Ability spec not found for: %s"), *Item->ItemAbility->GetName());
        return false;
    }

    // ✅ SourceObject를 설정하고 어빌리티 활성화
    UE_LOG(LogTemp, Warning, TEXT("⚡ Found ability spec, setting SourceObject to: %s"), *Item->GetName());
    Spec->SourceObject = Item;
    
    // ✅ 어빌리티 활성화 전에 아이템 정보 미리 저장
    FGameplayTag ConsumableTag = FGameplayTag::RequestGameplayTag("Item.Consumable");
    FGameplayTag TrapTag = FGameplayTag::RequestGameplayTag("Item.Trap");
    bool bShouldConsume = Item->ItemTag.MatchesTag(ConsumableTag) || Item->ItemTag.MatchesTag(TrapTag);
    
    bool bSuccess = ASC->TryActivateAbility(Spec->Handle);
    UE_LOG(LogTemp, Warning, TEXT("⚡ TryActivateAbility result: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    if (bSuccess && bShouldConsume)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚡ Ability activated successfully, processing item consumption"));
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
            
            // ✅ 개선된 시스템으로 이벤트 발생
            int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Item, SlotIndex);
            OnInventoryChanged.Broadcast(UnifiedIndex, nullptr);
            Item->Destroy();
        }
        else
        {
            // ✅ 개선된 시스템으로 이벤트 발생
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
        
        // ✅ 개선된 시스템으로 이벤트 발생
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
        
        // ✅ 개선된 시스템으로 이벤트 발생
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
        // ✅ 개선된 시스템으로 이벤트 발생
        int32 UnifiedIndex = UInventorySlotUtils::MakeSlotIndex(EInventorySlotType::Weapon, i);
        OnInventoryChanged.Broadcast(UnifiedIndex, WeaponSlots[i]);
    }
}

void UCYInventoryComponent::OnRep_ItemSlots()
{
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        // ✅ 개선된 시스템으로 이벤트 발생
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