#include "Abilities/GA_WeaponAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Player/CYPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GAS/CYAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_WeaponAttack::UGA_WeaponAttack()
{
    // 인스턴스 정책 설정
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;

    // 태그 설정
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Weapon.Attack"));
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Attacking"));
    
    // 블록 태그
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Stunned"));
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
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
    if (CooldownEffectClass)
    {
        const FGameplayTagContainer* CooldownTags = GetCooldownTags();
        if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
            return;
        }
    }

    // 공격 수행
    PerformAttack();

    // 쿨다운 적용
    if (CooldownEffectClass)
    {
        FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(CooldownEffectClass, 1);
        if (CooldownSpec.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_WeaponAttack::PerformAttack()
{
    ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!Character) return;

    // 라인 트레이스로 타겟 찾기
    FHitResult HitResult;
    if (Character->PerformLineTrace(HitResult))
    {
        if (AActor* Target = HitResult.GetActor())
        {
            // 타겟이 GAS를 가지고 있는지 확인
            UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
            if (TargetASC && DamageEffectClass)
            {
                // 데미지 계산
                UCYAttributeSet* SourceAttributes = Cast<UCYAttributeSet>(
                    GetAbilitySystemComponentFromActorInfo()->GetAttributeSet(UCYAttributeSet::StaticClass())
                );
                
                float Damage = 50.0f;  // 기본 데미지
                if (SourceAttributes)
                {
                    Damage = SourceAttributes->GetAttackPower();
                }

                // 데미지 이펙트 생성 및 적용
                FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
                EffectContext.AddSourceObject(Character);
                EffectContext.AddHitResult(HitResult);

                FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1);
                if (DamageSpec.IsValid())
                {
                    // SetByCaller로 데미지 설정
                    DamageSpec.Data.Get()->SetSetByCallerMagnitude(
                        FGameplayTag::RequestGameplayTag("Data.Damage"), 
                        Damage
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