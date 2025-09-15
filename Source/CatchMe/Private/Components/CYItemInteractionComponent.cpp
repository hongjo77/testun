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
    // ✅ Tick 비활성화
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYItemInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // ✅ 서버에서만 타이머 시작
    if (GetOwner()->HasAuthority())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ItemCheckTimer,
            this,
            &UCYItemInteractionComponent::CheckForNearbyItems,
            CheckInterval,
            true // 반복
        );
        
        UE_LOG(LogTemp, Warning, TEXT("✅ ItemInteractionComponent timer started (%.2fs interval)"), CheckInterval);
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
                    
                    // ✅ 강제로 픽업 시도
                    if (ACYTrapBase* Trap = Cast<ACYTrapBase>(Item))
                    {
                        if (Trap->TrapState == ETrapState::MapPlaced)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("🔧 Force picking up trap: %s"), *Item->GetName());
                            ServerPickupItem(Item);
                            return;
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("🔧 Force picking up item: %s"), *Item->GetName());
                        ServerPickupItem(Item);
                        return;
                    }
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
    if (!GetOwner()) return;
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    
    // ✅ ObjectType을 WorldDynamic으로 설정
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(GetOwner());
    
    TArray<AActor*> OutActors;
    bool bHit = UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        StartLocation,
        InteractionRange,
        ObjectTypes, // ✅ ObjectType 지정
        ACYItemBase::StaticClass(),
        IgnoreActors,
        OutActors
    );

    ACYItemBase* ClosestItem = nullptr;
    float ClosestDistance = FLT_MAX;

    UE_LOG(LogTemp, Verbose, TEXT("🔍 CheckForNearbyItems: Found %d actors"), OutActors.Num());

    if (bHit)
    {
        for (AActor* Actor : OutActors)
        {
            if (ACYItemBase* Item = Cast<ACYItemBase>(Actor))
            {
                UE_LOG(LogTemp, Verbose, TEXT("🔍 Found item: %s (PickedUp: %s)"), 
                       *Item->GetName(), Item->bIsPickedUp ? TEXT("true") : TEXT("false"));
                
                if (Item->bIsPickedUp) 
                {
                    continue;
                }
                
                // ✅ 트랩의 경우 상태 체크
                if (ACYTrapBase* Trap = Cast<ACYTrapBase>(Item))
                {
                    UE_LOG(LogTemp, Verbose, TEXT("🔍 Trap state: %s"), 
                           Trap->TrapState == ETrapState::MapPlaced ? TEXT("MapPlaced") : TEXT("PlayerPlaced"));
                    
                    if (Trap->TrapState != ETrapState::MapPlaced)
                    {
                        UE_LOG(LogTemp, Verbose, TEXT("🔍 Trap not pickupable"));
                        continue;
                    }
                }
                
                float Distance = FVector::Dist(StartLocation, Item->GetActorLocation());
                UE_LOG(LogTemp, Verbose, TEXT("🔍 Item %s at distance %f"), *Item->GetName(), Distance);
                
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestItem = Item;
                }
            }
        }
    }

    // ✅ 변경사항이 있을 때만 로그 출력
    if (NearbyItem != ClosestItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("🔍 Nearby item changed: %s -> %s (Distance: %.1f)"), 
               NearbyItem ? *NearbyItem->GetName() : TEXT("NULL"),
               ClosestItem ? *ClosestItem->GetName() : TEXT("NULL"),
               ClosestDistance);
               
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