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

    // ✅ 하드코딩된 태그 사용 (초기화 순서 문제 해결)
    FGameplayTag WeaponAttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Attack"));
    FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("State.Attacking"));
    FGameplayTag StunnedTag = FGameplayTag::RequestGameplayTag(FName("State.Stunned"));
    FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
    
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(WeaponAttackTag);
    SetAssetTags(AssetTags);
    
    FGameplayTagContainer OwnedTags;
    OwnedTags.AddTag(AttackingTag);
    ActivationOwnedTags = OwnedTags;
    
    FGameplayTagContainer BlockedTags;
    BlockedTags.AddTag(StunnedTag);
    BlockedTags.AddTag(DeadTag);
    ActivationBlockedTags = BlockedTags;
    
    UE_LOG(LogTemp, Warning, TEXT("✅ GA_WeaponAttack created with tag: %s"), *WeaponAttackTag.ToString());
}

void UGA_WeaponAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 쿨다운 체크
    if (IsOnCooldown(ActorInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 공격 수행
    PerformAttack();

    // 쿨다운 적용
    ApplyWeaponCooldown(Handle, ActorInfo, ActivationInfo);

    UE_LOG(LogTemp, Log, TEXT("Weapon attack completed"));
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
        ProcessHitTarget(HitResult);
    }
}

bool UGA_WeaponAttack::IsOnCooldown(const FGameplayAbilityActorInfo* ActorInfo) const
{
    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    return CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags);
}

void UGA_WeaponAttack::ProcessHitTarget(const FHitResult& HitResult)
{
    AActor* Target = HitResult.GetActor();
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    if (!TargetASC) return;

    ApplyDamageToTarget(TargetASC, HitResult);
}

void UGA_WeaponAttack::ApplyDamageToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult)
{
    FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
    EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
    EffectContext.AddHitResult(HitResult);

    FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(UGE_WeaponDamage::StaticClass(), 1);
    if (DamageSpec.IsValid())
    {
        GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
            *DamageSpec.Data.Get(),
            TargetASC
        );

        UE_LOG(LogTemp, Log, TEXT("Applied weapon damage to %s"), *HitResult.GetActor()->GetName());
    }
}

void UGA_WeaponAttack::ApplyWeaponCooldown(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(UGE_WeaponAttackCooldown::StaticClass(), 1);
    if (CooldownSpec.IsValid())
    {
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
    }
}