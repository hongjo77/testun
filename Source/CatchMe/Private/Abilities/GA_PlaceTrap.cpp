// GA_PlaceTrap.cpp - SourceObject 문제 해결
#include "Abilities/GA_PlaceTrap.h"
#include "Items/CYTrapFactory.h"
#include "Items/CYItemBase.h"
#include "Engine/World.h"
#include "GAS/CYGameplayEffects.h"
#include "CYGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYTrapBase.h"
#include "Components/CYInventoryComponent.h"

UGA_PlaceTrap::UGA_PlaceTrap()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated; // ✅ 서버에서만 실행

    // ✅ 하드코딩된 태그 사용
    FGameplayTag PlaceTrapTag = FGameplayTag::RequestGameplayTag(FName("Ability.Trap.Place"));
    FGameplayTag StunnedTag = FGameplayTag::RequestGameplayTag(FName("State.Stunned"));
    FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
    
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(PlaceTrapTag);
    SetAssetTags(AssetTags);
    
    FGameplayTagContainer BlockedTags;
    BlockedTags.AddTag(StunnedTag);
    BlockedTags.AddTag(DeadTag);
    ActivationBlockedTags = BlockedTags;
    
    UE_LOG(LogTemp, Warning, TEXT("✅ GA_PlaceTrap created with tag: %s"), *PlaceTrapTag.ToString());
}

void UGA_PlaceTrap::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Warning, TEXT("🚀 GA_PlaceTrap::ActivateAbility called"));
    
    if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GA_PlaceTrap: No authority or prediction key"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    AActor* OwnerActor = GetAvatarActorFromActorInfo();
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GA_PlaceTrap: OwnerActor is null"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("🚀 GA_PlaceTrap: OwnerActor found: %s"), *OwnerActor->GetName());

    // 쿨다운 체크
    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
    {
        UE_LOG(LogTemp, Warning, TEXT("⏰ GA_PlaceTrap: On cooldown"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // ✅ 새로운 방식: 인벤토리에서 직접 아이템 가져오기
    ACYItemBase* SourceItem = FindValidTrapItemInInventory(OwnerActor);
    
    if (!SourceItem)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GA_PlaceTrap: No valid trap item found in inventory"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("🚀 GA_PlaceTrap: Creating trap from item: %s"), 
           *SourceItem->ItemName.ToString());

    // 트랩 설치 위치 계산
    FVector SpawnLocation = CalculateSpawnLocation(OwnerActor);
    FRotator SpawnRotation = OwnerActor->GetActorRotation();
    
    UE_LOG(LogTemp, Warning, TEXT("🚀 GA_PlaceTrap: Spawn location: %s"), *SpawnLocation.ToString());

    // ✅ 팩토리를 통한 트랩 생성
    ACYTrapBase* NewTrap = UCYTrapFactory::CreateTrapFromItem(
        GetWorld(),
        SourceItem,
        SpawnLocation,
        SpawnRotation,
        OwnerActor,
        Cast<APawn>(OwnerActor)
    );

    if (NewTrap)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Trap successfully created: %s"), 
               *NewTrap->GetClass()->GetName());
        
        // ✅ 트랩 설치 후 아이템 소모 처리
        ConsumeItemFromInventory(OwnerActor, SourceItem);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to create trap from item: %s"), 
               *SourceItem->ItemName.ToString());
    }

    // 쿨다운 적용
    ApplyCooldown(Handle, ActorInfo, ActivationInfo);
    UE_LOG(LogTemp, Warning, TEXT("🚀 GA_PlaceTrap: Ability completed"));
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

ACYItemBase* UGA_PlaceTrap::FindValidTrapItemInInventory(AActor* OwnerActor)
{
    if (!OwnerActor) return nullptr;

    UCYInventoryComponent* InventoryComp = OwnerActor->FindComponentByClass<UCYInventoryComponent>();
    if (!InventoryComp) return nullptr;

    // 아이템 슬롯에서 트랩 아이템 찾기
    for (ACYItemBase* Item : InventoryComp->ItemSlots)
    {
        if (Item && Item->ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag("Item.Trap")) && Item->ItemCount > 0)
        {
            return Item;
        }
    }

    return nullptr;
}

void UGA_PlaceTrap::ConsumeItemFromInventory(AActor* OwnerActor, ACYItemBase* SourceItem)
{
    if (!OwnerActor || !SourceItem) return;

    UCYInventoryComponent* InventoryComp = OwnerActor->FindComponentByClass<UCYInventoryComponent>();
    if (!InventoryComp) return;

    // 아이템 수량 감소
    SourceItem->ItemCount--;
    
    if (SourceItem->ItemCount <= 0)
    {
        // 아이템이 모두 소모되면 슬롯에서 제거
        for (int32 i = 0; i < InventoryComp->ItemSlots.Num(); ++i)
        {
            if (InventoryComp->ItemSlots[i] == SourceItem)
            {
                InventoryComp->ItemSlots[i] = nullptr;
                
                // 이벤트 발생
                int32 UnifiedIndex = i; // ItemSlot은 0부터 시작
                InventoryComp->OnInventoryChanged.Broadcast(UnifiedIndex, nullptr);
                
                SourceItem->Destroy();
                break;
            }
        }
    }
    else
    {
        // 수량만 감소한 경우 이벤트 발생
        for (int32 i = 0; i < InventoryComp->ItemSlots.Num(); ++i)
        {
            if (InventoryComp->ItemSlots[i] == SourceItem)
            {
                int32 UnifiedIndex = i;
                InventoryComp->OnInventoryChanged.Broadcast(UnifiedIndex, SourceItem);
                break;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("🔧 Consumed trap item: %s (Remaining: %d)"), 
           *SourceItem->ItemName.ToString(), SourceItem->ItemCount);
}

FVector UGA_PlaceTrap::CalculateSpawnLocation(AActor* OwnerActor)
{
    if (!OwnerActor)
    {
        return FVector::ZeroVector;
    }

    FHitResult HitResult;
    UCYWeaponComponent* WeaponComp = OwnerActor->FindComponentByClass<UCYWeaponComponent>();
    
    // 라인 트레이스로 정확한 위치 계산
    if (WeaponComp && WeaponComp->PerformLineTrace(HitResult, 300.0f))
    {
        return HitResult.Location;
    }

    // 백업: 캐릭터 앞쪽에 배치
    FVector ForwardLocation = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 200.0f;
    ForwardLocation.Z = OwnerActor->GetActorLocation().Z;
    return ForwardLocation;
}

void UGA_PlaceTrap::ApplyTrapCooldown(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(UGE_TrapPlaceCooldown::StaticClass(), 1);
    if (CooldownSpec.IsValid())
    {
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
    }
}