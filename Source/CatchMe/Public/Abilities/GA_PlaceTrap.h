#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlaceTrap.generated.h"

class ACYTrapBase;
struct FGameplayEventData;

UCLASS()
class CATCHME_API UGA_PlaceTrap : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlaceTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	TSubclassOf<ACYTrapBase> TrapClass;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// 트랩 배치 위치 계산
	FVector CalculateSpawnLocation(AActor* OwnerActor);

	// 트랩 효과 설정 (Event + SourceObject 이중 보장)
	void ConfigureTrapEffects(ACYTrapBase* Trap, const FGameplayEventData* TriggerEventData);

	// 쿨다운 적용
	void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo);
};