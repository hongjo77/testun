#include "GAS/CYGameplayEffects.h"
#include "GAS/CYAttributeSet.h"

UGE_ImmobilizeTrap::UGE_ImmobilizeTrap()
{
    // 기본 설정
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(5.0f); // 5초
    
    // 이동속도를 0으로 설정하는 Modifier 추가
    FGameplayModifierInfo MoveSpeedModifier;
    MoveSpeedModifier.Attribute = UCYAttributeSet::GetMoveSpeedAttribute();
    MoveSpeedModifier.ModifierOp = EGameplayModOp::Override;
    MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(0.0f);
    
    Modifiers.Add(MoveSpeedModifier);
    
    // 태그 설정 (태그가 있을 때만)
    // InheritableOwnedTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Effect.Debuff.Immobilize"));
    // InheritableGrantedTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("State.Immobilized"));
}

UGE_WeaponDamage::UGE_WeaponDamage()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;
    
    FGameplayModifierInfo HealthModifier;
    HealthModifier.Attribute = UCYAttributeSet::GetHealthAttribute();
    HealthModifier.ModifierOp = EGameplayModOp::Additive;
    
    // 임시로 고정값 사용 (태그 문제 회피)
    HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(-50.0f); // 고정 데미지
    
    Modifiers.Add(HealthModifier);
}

UGE_WeaponAttackCooldown::UGE_WeaponAttackCooldown()
{
    // 1초 지속
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(1.0f);
    
    // 쿨다운 태그
    // InheritableOwnedTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Cooldown.Weapon.Attack"));
    // InheritableGrantedTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Cooldown.Weapon.Attack"));
}

UGE_TrapPlaceCooldown::UGE_TrapPlaceCooldown()
{
    // 3초 지속
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(3.0f);
    
    // 쿨다운 태그
    // InheritableOwnedTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Cooldown.Trap.Place"));
    // InheritableGrantedTagsContainer.AddTag(FGameplayTag::RequestGameplayTag("Cooldown.Trap.Place"));
}

UGE_InitialStats::UGE_InitialStats()
{
    // 영구 지속
    DurationPolicy = EGameplayEffectDurationType::Infinite;
    
    // Health 설정
    FGameplayModifierInfo HealthModifier;
    HealthModifier.Attribute = UCYAttributeSet::GetHealthAttribute();
    HealthModifier.ModifierOp = EGameplayModOp::Additive;
    HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(100.0f);
    Modifiers.Add(HealthModifier);
    
    // MaxHealth 설정
    FGameplayModifierInfo MaxHealthModifier;
    MaxHealthModifier.Attribute = UCYAttributeSet::GetMaxHealthAttribute();
    MaxHealthModifier.ModifierOp = EGameplayModOp::Additive;
    MaxHealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(100.0f);
    Modifiers.Add(MaxHealthModifier);
    
    // MoveSpeed 설정
    FGameplayModifierInfo MoveSpeedModifier;
    MoveSpeedModifier.Attribute = UCYAttributeSet::GetMoveSpeedAttribute();
    MoveSpeedModifier.ModifierOp = EGameplayModOp::Additive;
    MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(600.0f);
    Modifiers.Add(MoveSpeedModifier);
    
    // AttackPower 설정
    FGameplayModifierInfo AttackPowerModifier;
    AttackPowerModifier.Attribute = UCYAttributeSet::GetAttackPowerAttribute();
    AttackPowerModifier.ModifierOp = EGameplayModOp::Additive;
    AttackPowerModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(50.0f);
    Modifiers.Add(AttackPowerModifier);
}