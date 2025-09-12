#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_WeaponAttack.generated.h"

class UAbilitySystemComponent;

UCLASS()
class CATCHME_API UGA_WeaponAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponAttack();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 핵심 로직 함수들
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void PerformAttack();

	// 유틸리티 함수들
	bool IsOnCooldown(const FGameplayAbilityActorInfo* ActorInfo) const;
	void ProcessHitTarget(const FHitResult& HitResult);
	void ApplyDamageToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult);
	void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo);
};