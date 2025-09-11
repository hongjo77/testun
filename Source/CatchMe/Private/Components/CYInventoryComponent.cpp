#include "Components/CYInventoryComponent.h"
#include "Items/CYItemBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CYGameplayTags.h"
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

bool UCYInventoryComponent::AddItemWithStacking(ACYItemBase* Item)
{
    if (!Item) return false;

    // 1. 스택 가능한 슬롯 찾기
    int32 StackableSlot = FindStackableItemSlot(Item);
    if (StackableSlot != -1)
    {
        // 스택에 추가
        ACYItemBase* ExistingItem = ItemSlots[StackableSlot];
        int32 AddableCount = FMath::Min(Item->ItemCount, 
                                      ExistingItem->MaxStackCount - ExistingItem->ItemCount);
        
        ExistingItem->ItemCount += AddableCount;
        Item->ItemCount -= AddableCount;
        
        OnInventoryChanged.Broadcast(StackableSlot, ExistingItem);
        
        // 아이템이 모두 스택되었으면 원본 아이템 제거
        if (Item->ItemCount <= 0)
        {
            Item->Destroy();
            return true;
        }
    }

    // 2. 새 슬롯에 추가 (남은 수량이 있다면)
    int32 EmptySlot = FindEmptyItemSlot();
    if (EmptySlot != -1)
    {
        ItemSlots[EmptySlot] = Item;
        OnInventoryChanged.Broadcast(EmptySlot, Item);
        return true;
    }

    return false; // 인벤토리 가득참
}

bool UCYInventoryComponent::AddWeapon(ACYItemBase* Weapon)
{
    if (!Weapon) return false;

    int32 EmptySlot = FindEmptyWeaponSlot();
    if (EmptySlot != -1)
    {
        WeaponSlots[EmptySlot] = Weapon;
        OnInventoryChanged.Broadcast(EmptySlot + 1000, Weapon); // 무기는 1000번대로 구분
        return true;
    }

    return false; // 무기 슬롯 가득참
}

bool UCYInventoryComponent::AddItem(ACYItemBase* Item, int32 SlotIndex)
{
    if (!Item) return false;

    // ✅ 무기와 아이템을 자동으로 구분해서 추가
    const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
    
    if (Item->ItemTag.MatchesTag(GameplayTags.Item_Weapon))
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

bool UCYInventoryComponent::UseItem(int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority()) return false;

    ACYItemBase* Item = GetItem(SlotIndex);
    if (!Item) return false;

    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC || !Item->ItemAbility) return false;

    FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(Item->ItemAbility);
    if (Spec)
    {
        Spec->SourceObject = Item;
        
        bool bSuccess = ASC->TryActivateAbility(Spec->Handle);
        
        const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
        
        if (bSuccess && (Item->ItemTag.MatchesTag(GameplayTags.Item_Consumable) ||
                        Item->ItemTag.MatchesTag(GameplayTags.Item_Trap)))
        {
            // ✅ 수량 감소 로직
            Item->ItemCount--;
            if (Item->ItemCount <= 0)
            {
                // 수량이 0이 되면 슬롯에서 제거
                if (SlotIndex >= 1000)
                {
                    WeaponSlots[SlotIndex - 1000] = nullptr;
                }
                else
                {
                    ItemSlots[SlotIndex] = nullptr;
                }
                OnInventoryChanged.Broadcast(SlotIndex, nullptr);
                Item->Destroy();
            }
            else
            {
                OnInventoryChanged.Broadcast(SlotIndex, Item);
            }
        }
        
        return bSuccess;
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