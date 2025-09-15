#include "Abilities/GA_PlaceTrap.h"
#include "Items/CYTrapFactory.h"
#include "Items/CYItemBase.h"
#include "Engine/World.h"
#include "GAS/CYGameplayEffects.h"
#include "CYGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYTrapBase.h"

UGA_PlaceTrap::UGA_PlaceTrap()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
    
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(GameplayTags.Ability_Trap_Place);
    SetAssetTags(AssetTags);
    
    FGameplayTagContainer BlockedTags;
    BlockedTags.AddTag(GameplayTags.State_Stunned);
    BlockedTags.AddTag(GameplayTags.State_Dead);
    ActivationBlockedTags = BlockedTags;
}

void UGA_PlaceTrap::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
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

    // 쿨다운 체크
    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
    {
        UE_LOG(LogTemp, Warning, TEXT("⏰ GA_PlaceTrap: On cooldown"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // ✅ 단순한 SourceObject 방식만 사용
    ACYItemBase* SourceItem = nullptr;
    const FGameplayAbilitySpec* CurrentSpec = GetCurrentAbilitySpec();
    if (CurrentSpec && CurrentSpec->SourceObject.IsValid())
    {
        SourceItem = Cast<ACYItemBase>(CurrentSpec->SourceObject.Get());
    }

    if (!SourceItem)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GA_PlaceTrap: No valid source item found"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("🚀 GA_PlaceTrap: Creating trap from item: %s"), 
           *SourceItem->ItemName.ToString());

    // 트랩 설치 위치 계산
    FVector SpawnLocation = CalculateSpawnLocation(OwnerActor);
    FRotator SpawnRotation = OwnerActor->GetActorRotation();

    // ✅ 팩토리를 통한 트랩 생성 (완전히 위임)
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
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to create trap from item: %s"), 
               *SourceItem->ItemName.ToString());
    }

    // 쿨다운 적용
    ApplyCooldown(Handle, ActorInfo, ActivationInfo);
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
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