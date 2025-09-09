// Systems/CYTrapManagerComponent.cpp
#include "Systems/CYTrapManagerComponent.h"
#include "Item/CYTrapBase.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UCYTrapManagerComponent::UCYTrapManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYTrapManagerComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCYTrapManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UCYTrapManagerComponent, PlacedTraps);
}

void UCYTrapManagerComponent::ServerPlaceTrap_Implementation(TSubclassOf<ACYTrapBase> TrapClass, FVector Location, FRotator Rotation)
{
    if (!GetOwner()->HasAuthority() || !TrapClass) return;

    // 트랩 개수 제한 체크
    CleanupTraps();
    if (PlacedTraps.Num() >= MaxTraps)
    {
        ServerRemoveOldestTrap();
    }

    // 새 트랩 생성
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.Instigator = Cast<APawn>(GetOwner());

    ACYTrapBase* NewTrap = GetWorld()->SpawnActor<ACYTrapBase>(TrapClass, Location, Rotation, SpawnParams);
    if (NewTrap)
    {
        PlacedTraps.Add(NewTrap);
        OnTrapPlaced.Broadcast(NewTrap);
        
        UE_LOG(LogTemp, Warning, TEXT("Placed trap: %s"), *NewTrap->GetName());
    }
}

void UCYTrapManagerComponent::ServerRemoveOldestTrap_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

    CleanupTraps();
    if (PlacedTraps.Num() > 0)
    {
        ACYTrapBase* OldestTrap = PlacedTraps[0];
        PlacedTraps.RemoveAt(0);
        
        if (OldestTrap)
        {
            OnTrapRemoved.Broadcast(OldestTrap);
            OldestTrap->Destroy();
        }
    }
}

void UCYTrapManagerComponent::ServerRemoveTrap_Implementation(ACYTrapBase* Trap)
{
    if (!GetOwner()->HasAuthority() || !Trap) return;

    PlacedTraps.Remove(Trap);
    OnTrapRemoved.Broadcast(Trap);
    Trap->Destroy();
}

void UCYTrapManagerComponent::ServerClearAllTraps_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

    for (ACYTrapBase* Trap : PlacedTraps)
    {
        if (Trap)
        {
            OnTrapRemoved.Broadcast(Trap);
            Trap->Destroy();
        }
    }
    
    PlacedTraps.Empty();
}

void UCYTrapManagerComponent::CleanupTraps()
{
    // 널 포인터나 파괴된 트랩들 제거
    for (int32 i = PlacedTraps.Num() - 1; i >= 0; i--)
    {
        if (!PlacedTraps[i] || !IsValid(PlacedTraps[i]))
        {
            PlacedTraps.RemoveAt(i);
        }
    }
}