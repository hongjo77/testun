#include "Components/CYItemInteractionComponent.h"
#include "Components/CYInventoryComponent.h"
#include "Components/CYWeaponComponent.h"
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
    UE_LOG(LogTemp, Warning, TEXT("🔧 InteractWithNearbyItem called! NearbyItem: %s"), 
           NearbyItem ? *NearbyItem->GetName() : TEXT("NULL"));
    
    if (NearbyItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("🔧 Calling ServerPickupItem for: %s"), *NearbyItem->GetName());
        ServerPickupItem(NearbyItem);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No nearby item to interact with!"));
    }
}

void UCYItemInteractionComponent::ServerPickupItem_Implementation(ACYItemBase* Item)
{
    UE_LOG(LogTemp, Warning, TEXT("🔧 ServerPickupItem called for: %s"), 
           Item ? *Item->GetName() : TEXT("NULL"));
    
    if (!Item || !GetOwner()->HasAuthority() || Item->bIsPickedUp) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ServerPickupItem failed - Item: %s, HasAuthority: %s, bIsPickedUp: %s"), 
               Item ? TEXT("Valid") : TEXT("NULL"),
               GetOwner()->HasAuthority() ? TEXT("true") : TEXT("false"),
               Item ? (Item->bIsPickedUp ? TEXT("true") : TEXT("false")) : TEXT("N/A"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("🔧 ServerPickupItem: %s with tag %s"), 
           *Item->ItemName.ToString(), *Item->ItemTag.ToString());

    UCYInventoryComponent* InventoryComp = GetInventoryComponent();
    if (!InventoryComp) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No InventoryComponent found"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("🔧 InventoryComponent found, calling AddItem..."));
    
    // ✅ 단순하게 인벤토리에만 추가 (AddItem에서 자동으로 무기/아이템 구분)
    bool bAddedToInventory = InventoryComp->AddItem(Item);
    UE_LOG(LogTemp, Warning, TEXT("🔧 AddItem result: %s"), 
           bAddedToInventory ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    if (bAddedToInventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Item successfully added to inventory, calling OnPickup..."));
        Item->OnPickup(Cast<ACYPlayerCharacter>(GetOwner()));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to add item to inventory!"));
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