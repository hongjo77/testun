#include "Components/CYItemInteractionComponent.h"
#include "Components/CYInventoryComponent.h"
#include "Items/CYItemBase.h"
#include "Items/CYWeaponBase.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/CYPlayerCharacter.h"

UCYItemInteractionComponent::UCYItemInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UCYItemInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCYItemInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // 서버에서만 근처 아이템 체크
    if (GetOwner()->HasAuthority())
    {
        CheckForNearbyItems();
    }
}

void UCYItemInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCYItemInteractionComponent, NearbyItem);
}

void UCYItemInteractionComponent::InteractWithNearbyItem()
{
    if (NearbyItem)
    {
        ServerPickupItem(NearbyItem);
    }
}

void UCYItemInteractionComponent::ServerPickupItem_Implementation(ACYItemBase* Item)
{
    if (!Item || !GetOwner()->HasAuthority() || Item->bIsPickedUp) return;

    UCYInventoryComponent* InventoryComp = GetInventoryComponent();
    if (!InventoryComp) return;

    // 무기인 경우 특별 처리 (장착)
    if (ACYWeaponBase* Weapon = Cast<ACYWeaponBase>(Item))
    {
        // 무기 장착 로직은 별도 컴포넌트에서 처리
        // 여기서는 인벤토리에 추가만
        if (InventoryComp->AddItem(Item))
        {
            Item->OnPickup(Cast<ACYPlayerCharacter>(GetOwner()));
        }
    }
    else
    {
        // 일반 아이템
        if (InventoryComp->AddItem(Item))
        {
            Item->OnPickup(Cast<ACYPlayerCharacter>(GetOwner()));
        }
    }
}

void UCYItemInteractionComponent::CheckForNearbyItems()
{
    FVector StartLocation = GetOwner()->GetActorLocation();
    
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(GetOwner());
    
    TArray<AActor*> OutActors;
    bool bHit = UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        StartLocation,
        InteractionRange,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        ACYItemBase::StaticClass(),
        IgnoreActors,
        OutActors
    );

    ACYItemBase* ClosestItem = nullptr;
    float ClosestDistance = FLT_MAX;

    if (bHit)
    {
        for (AActor* Actor : OutActors)
        {
            if (ACYItemBase* Item = Cast<ACYItemBase>(Actor))
            {
                if (Item->bIsPickedUp) continue;
                
                float Distance = FVector::Dist(StartLocation, Item->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestItem = Item;
                }
            }
        }
    }

    if (NearbyItem != ClosestItem)
    {
        NearbyItem = ClosestItem;
        OnRep_NearbyItem();
    }
}

void UCYItemInteractionComponent::OnRep_NearbyItem()
{
    OnNearbyItemChanged.Broadcast(NearbyItem, NearbyItem != nullptr);
}

UCYInventoryComponent* UCYItemInteractionComponent::GetInventoryComponent() const
{
    return GetOwner()->FindComponentByClass<UCYInventoryComponent>();
}