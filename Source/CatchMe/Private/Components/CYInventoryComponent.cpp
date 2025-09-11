#include "Components/CYInventoryComponent.h"
#include "Items/CYItemBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"

UCYInventoryComponent::UCYInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 인벤토리 크기 설정
    InventorySlots.SetNum(InventorySize);
}

void UCYInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCYInventoryComponent, InventorySlots);
}

int32 UCYInventoryComponent::FindEmptySlot() const
{
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (!InventorySlots[i])
        {
            return i;
        }
    }
    return -1;
}

bool UCYInventoryComponent::AddItem(ACYItemBase* Item, int32 SlotIndex)
{
    if (!Item) return false;

    // 슬롯 인덱스가 지정되지 않으면 빈 슬롯 찾기
    if (SlotIndex == -1)
    {
        SlotIndex = FindEmptySlot();
    }

    // 유효한 슬롯인지 확인
    if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num()) return false;
    if (InventorySlots[SlotIndex]) return false; // 이미 차있음

    // 아이템 추가
    InventorySlots[SlotIndex] = Item;
    OnInventoryChanged.Broadcast(SlotIndex, Item);
    
    return true;
}

bool UCYInventoryComponent::RemoveItem(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num()) return false;
    if (!InventorySlots[SlotIndex]) return false;

    InventorySlots[SlotIndex] = nullptr;
    OnInventoryChanged.Broadcast(SlotIndex, nullptr);
    
    return true;
}

ACYItemBase* UCYInventoryComponent::GetItem(int32 SlotIndex) const
{
    if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num()) return nullptr;
    return InventorySlots[SlotIndex];
}

bool UCYInventoryComponent::UseItem(int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority()) return false;

    ACYItemBase* Item = GetItem(SlotIndex);
    if (!Item) return false;

    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC || !Item->ItemAbility) return false;

    // 어빌리티 찾기 및 실행
    FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(Item->ItemAbility);
    if (Spec)
    {
        // 소스 오브젝트 설정
        Spec->SourceObject = Item;
        
        bool bSuccess = ASC->TryActivateAbility(Spec->Handle);
        
        // 소모품이면 제거
        if (bSuccess && (Item->ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag("Item.Consumable")) ||
                        Item->ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag("Item.Trap"))))
        {
            RemoveItem(SlotIndex);
        }
        
        return bSuccess;
    }
    
    return false;
}

void UCYInventoryComponent::ServerUseItem_Implementation(int32 SlotIndex)
{
    UseItem(SlotIndex);
}

void UCYInventoryComponent::OnRep_Inventory()
{
    // UI 업데이트 등
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        OnInventoryChanged.Broadcast(i, InventorySlots[i]);
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