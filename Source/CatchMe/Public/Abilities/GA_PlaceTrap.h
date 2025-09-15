#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlaceTrap.generated.h"

class ACYTrapBase;

/**
 * 대폭 단순화된 트랩 배치 어빌리티
 * 팩토리 패턴을 사용하여 결합도를 낮춤
 */
UCLASS()
class CATCHME_API UGA_PlaceTrap : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlaceTrap();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 트랩 배치 위치 계산
	UFUNCTION(BlueprintCallable, Category = "Trap")
	FVector CalculateSpawnLocation(AActor* OwnerActor);

	// 쿨다운 적용
	void ApplyTrapCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo);
};