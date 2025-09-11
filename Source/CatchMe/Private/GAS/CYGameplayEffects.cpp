#include "GAS/CYGameplayEffects.h"
#include "GAS/CYAttributeSet.h"

// 커스텀 이동속도 조절 효과
UGE_MovementModifier::UGE_MovementModifier()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(5.0f); // 기본 5초
    
    // 이동속도 수정자 - 백분율 방식
    FGameplayModifierInfo MoveSpeedModifier;
    MoveSpeedModifier.Attribute = UCYAttributeSet::GetMoveSpeedAttribute();
    MoveSpeedModifier.ModifierOp = EGameplayModOp::Override; // Override 사용
    MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(0.0f); // 기본값 (커스텀에서 설정)
    
    Modifiers.Add(MoveSpeedModifier);
}

// 기존 완전 정지 트랩 (백업용)
UGE_ImmobilizeTrap::UGE_ImmobilizeTrap()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(5.0f);
    
    FGameplayModifierInfo MoveSpeedModifier;
    MoveSpeedModifier.Attribute = UCYAttributeSet::GetMoveSpeedAttribute();
    MoveSpeedModifier.ModifierOp = EGameplayModOp::Override;
    MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(0.0f); // 완전 정지
    
    Modifiers.Add(MoveSpeedModifier);
}

UGE_SlowTrap::UGE_SlowTrap()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(5.0f);
    
    FGameplayModifierInfo MoveSpeedModifier;
    MoveSpeedModifier.Attribute = UCYAttributeSet::GetMoveSpeedAttribute();
    MoveSpeedModifier.ModifierOp = EGameplayModOp::Override;
    MoveSpeedModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(50.0f); // 느림
    
    Modifiers.Add(MoveSpeedModifier);
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