#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_WeaponAttack.generated.h"

UCLASS()
class CATCHME_API UGA_WeaponAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponAttack();

	// Damage Effect
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	// Cooldown Effect
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	TSubclassOf<UGameplayEffect> CooldownEffectClass;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	void PerformAttack();
};