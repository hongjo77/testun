#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlaceTrap.generated.h"

UCLASS()
class CATCHME_API UGA_PlaceTrap : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlaceTrap();

	UPROPERTY(EditDefaultsOnly, Category = "Trap")
	TSubclassOf<class ACYTrapBase> TrapClass;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> TrapEffectClass;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};