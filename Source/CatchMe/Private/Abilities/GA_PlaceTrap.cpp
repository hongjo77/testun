#include "Abilities/GA_PlaceTrap.h"
#include "Items/CYTrapBase.h"
#include "Engine/World.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "GAS/CYGameplayEffects.h"
#include "CYGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Components/CYWeaponComponent.h"

UGA_PlaceTrap::UGA_PlaceTrap()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    TrapClass = ACYTrapBase::StaticClass();
    
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
    
    if (!OwnerActor || !TrapClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaceTrap: Missing OwnerActor or TrapClass"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 쿨다운 체크
    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 트랩 설치 위치 계산
    FHitResult HitResult;
    FVector SpawnLocation;
    
    UCYWeaponComponent* WeaponComp = OwnerActor->FindComponentByClass<UCYWeaponComponent>();
    if (WeaponComp && WeaponComp->PerformLineTrace(HitResult, 300.0f))
    {
        SpawnLocation = HitResult.Location;
    }
    else
    {
        SpawnLocation = OwnerActor->GetActorLocation() + 
                    OwnerActor->GetActorForwardVector() * 200.0f;
        SpawnLocation.Z = OwnerActor->GetActorLocation().Z;
    }

    FRotator SpawnRotation = OwnerActor->GetActorRotation();

    // 트랩 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerActor;
    SpawnParams.Instigator = Cast<APawn>(OwnerActor);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    if (ACYTrapBase* Trap = GetWorld()->SpawnActor<ACYTrapBase>(TrapClass, SpawnLocation, SpawnRotation, SpawnParams))
    {
        // ✅ SourceObject에서 DesiredTrapEffects 가져와서 적용
        if (const FGameplayAbilitySpec* CurrentSpec = GetCurrentAbilitySpec())
        {
            if (CurrentSpec->SourceObject.IsValid())
            {
                if (ACYItemBase* UsedItem = Cast<ACYItemBase>(CurrentSpec->SourceObject.Get()))
                {
                    if (UsedItem->DesiredTrapEffects.Num() > 0)
                    {
                        Trap->ItemEffects = UsedItem->DesiredTrapEffects;
                        UE_LOG(LogTemp, Warning, TEXT("Trap configured with %d custom effects from %s"), 
                               UsedItem->DesiredTrapEffects.Num(), *UsedItem->ItemName.ToString());
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("No DesiredTrapEffects in %s, using default"), 
                               *UsedItem->ItemName.ToString());
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Trap placed with %d effects"), Trap->ItemEffects.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn trap"));
    }

    // 쿨다운 적용
    FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(UGE_TrapPlaceCooldown::StaticClass(), 1);
    if (CooldownSpec.IsValid())
    {
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}