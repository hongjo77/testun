#include "Components/CYItemInteractionComponent.h"
#include "Components/CYInventoryComponent.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYItemBase.h"
#include "Items/CYWeaponBase.h"
#include "Engine/World.h"
#include "Items/CYTrapBase.h"
#include "Kismet/GameplayStatics.h"
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
        
        // ✅ 수동으로 근처 아이템 찾기 (디버깅용)
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACYItemBase::StaticClass(), FoundActors);
        
        FVector PlayerLocation = GetOwner()->GetActorLocation();
        for (AActor* Actor : FoundActors)
        {
            if (ACYItemBase* Item = Cast<ACYItemBase>(Actor))
            {
                float Distance = FVector::Dist(PlayerLocation, Item->GetActorLocation());
                if (Distance < InteractionRange && !Item->bIsPickedUp)
                {
                    UE_LOG(LogTemp, Warning, TEXT("🔧 Manual found nearby item: %s at distance %f"), 
                           *Item->GetName(), Distance);
                }
            }
        }
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

    UE_LOG(LogTemp, VeryVerbose, TEXT("🔍 CheckForNearbyItems: Found %d actors"), OutActors.Num());

    if (bHit)
    {
        for (AActor* Actor : OutActors)
        {
            if (ACYItemBase* Item = Cast<ACYItemBase>(Actor))
            {
                UE_LOG(LogTemp, VeryVerbose, TEXT("🔍 Checking item: %s (PickedUp: %s)"), 
                       *Item->GetName(), Item->bIsPickedUp ? TEXT("true") : TEXT("false"));
                
                if (Item->bIsPickedUp) 
                {
                    continue;
                }
                
                // ✅ 트랩의 경우 추가 체크
                if (ACYTrapBase* Trap = Cast<ACYTrapBase>(Item))
                {
                    UE_LOG(LogTemp, VeryVerbose, TEXT("🔍 Trap state: %s"), 
                           Trap->TrapState == ETrapState::MapPlaced ? TEXT("MapPlaced") : TEXT("PlayerPlaced"));
                    
                    // 플레이어가 설치한 트랩은 픽업 불가
                    if (Trap->TrapState != ETrapState::MapPlaced)
                    {
                        UE_LOG(LogTemp, VeryVerbose, TEXT("🔍 Trap not pickupable (PlayerPlaced)"));
                        continue;
                    }
                }
                
                float Distance = FVector::Dist(StartLocation, Item->GetActorLocation());
                UE_LOG(LogTemp, VeryVerbose, TEXT("🔍 Item %s at distance %f"), *Item->GetName(), Distance);
                
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
        UE_LOG(LogTemp, Warning, TEXT("🔍 Nearby item changed: %s -> %s"), 
               NearbyItem ? *NearbyItem->GetName() : TEXT("NULL"),
               ClosestItem ? *ClosestItem->GetName() : TEXT("NULL"));
               
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