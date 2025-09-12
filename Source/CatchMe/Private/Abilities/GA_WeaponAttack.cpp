#include "Abilities/GA_WeaponAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/CYWeaponComponent.h"
#include "GAS/CYAttributeSet.h"
#include "GAS/CYGameplayEffects.h"
#include "CYGameplayTags.h"

UGA_WeaponAttack::UGA_WeaponAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();

    // 🔥 AbilityTags deprecated 경고 해결 - SetAssetTags 사용
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(GameplayTags.Ability_Weapon_Attack);
    SetAssetTags(AssetTags);
    
    // Activation Owned Tags
    FGameplayTagContainer OwnedTags;
    OwnedTags.AddTag(GameplayTags.State_Attacking);
    ActivationOwnedTags = OwnedTags;
    
    // Activation Blocked Tags
    FGameplayTagContainer BlockedTags;
    BlockedTags.AddTag(GameplayTags.State_Stunned);
    BlockedTags.AddTag(GameplayTags.State_Dead);
    ActivationBlockedTags = BlockedTags;
}

void UGA_WeaponAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Warning, TEXT("UGA_WeaponAttack::ActivateAbility called"));
    
    if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponAttack: No authority or prediction key - ending ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponAttack: Authority check passed"));

    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponAttack: On cooldown - ending ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponAttack: Performing attack"));
    PerformAttack();

    FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(UGE_WeaponAttackCooldown::StaticClass(), 1);
    if (CooldownSpec.IsValid())
    {
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponAttack: Ability completed"));
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_WeaponAttack::PerformAttack()
{
    AActor* OwnerActor = GetAvatarActorFromActorInfo();
    if (!OwnerActor) return;

    UCYWeaponComponent* WeaponComp = OwnerActor->FindComponentByClass<UCYWeaponComponent>();
    if (!WeaponComp) return;

    FHitResult HitResult;
    if (WeaponComp->PerformLineTrace(HitResult))
    {
        if (AActor* Target = HitResult.GetActor())
        {
            UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
            if (TargetASC)
            {
                float Damage = 50.0f;
                
                if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
                {
                    FGameplayAttribute AttackPowerAttribute = UCYAttributeSet::GetAttackPowerAttribute();
                    if (SourceASC->HasAttributeSetForAttribute(AttackPowerAttribute))
                    {
                        Damage = SourceASC->GetNumericAttribute(AttackPowerAttribute);
                    }
                }

                FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
                EffectContext.AddSourceObject(OwnerActor);
                EffectContext.AddHitResult(HitResult);

                FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(UGE_WeaponDamage::StaticClass(), 1);
                if (DamageSpec.IsValid())
                {
                    GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
                        *DamageSpec.Data.Get(),
                        TargetASC
                    );

                    UE_LOG(LogTemp, Warning, TEXT("Applied %f damage to %s"), Damage, *Target->GetName());
                }
            }
        }
    }
}