#include "Abilities/GA_WeaponAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Player/CYPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Components/CYWeaponComponent.h"
#include "GAS/CYAttributeSet.h"
#include "GAS/CYGameplayEffects.h"  // ← 추가 필요

UGA_WeaponAttack::UGA_WeaponAttack()
{
    // 인스턴스 정책 설정
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Weapon.Attack"));
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Attacking"));
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Stunned"));
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
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

    // 쿨다운 체크 - C++ 클래스 직접 사용
    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponAttack: On cooldown - ending ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponAttack: Performing attack"));
    // 공격 수행
    PerformAttack();

    // 쿨다운 적용
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

    // 라인 트레이스로 타겟 찾기
    FHitResult HitResult;
    if (WeaponComp->PerformLineTrace(HitResult))
    {
        if (AActor* Target = HitResult.GetActor())
        {
            // 타겟이 GAS를 가지고 있는지 확인
            UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
            if (TargetASC)
            {
                // 데미지 계산
                float Damage = 50.0f;  // 기본 데미지
                
                if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
                {
                    FGameplayAttribute AttackPowerAttribute = UCYAttributeSet::GetAttackPowerAttribute();
                    if (SourceASC->HasAttributeSetForAttribute(AttackPowerAttribute))
                    {
                        Damage = SourceASC->GetNumericAttribute(AttackPowerAttribute);
                    }
                }

                // ✅ C++로 만든 데미지 GameplayEffect 사용
                FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
                EffectContext.AddSourceObject(PlayerCharacter);
                EffectContext.AddHitResult(HitResult);

                FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(UGE_WeaponDamage::StaticClass(), 1);
                if (DamageSpec.IsValid())
                {
                    // SetByCaller로 데미지 설정 (음수로 체력 감소)
                    DamageSpec.Data.Get()->SetSetByCallerMagnitude(
                        FGameplayTag::RequestGameplayTag("Data.Damage"), 
                        -Damage  // 음수로 체력 감소
                    );

                    // 타겟에 데미지 적용
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