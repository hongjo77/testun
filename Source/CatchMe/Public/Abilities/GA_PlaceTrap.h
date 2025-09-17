// GA_PlaceTrap.h - 새로운 함수 선언 추가
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlaceTrap.generated.h"

class ACYTrapBase;
class ACYItemBase;

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

	// ✅ 새로운 함수들 - 인벤토리에서 직접 아이템 관리
	UFUNCTION(BlueprintCallable, Category = "Trap")
	ACYItemBase* FindValidTrapItemInInventory(AActor* OwnerActor);

	UFUNCTION(BlueprintCallable, Category = "Trap")
	void ConsumeItemFromInventory(AActor* OwnerActor, ACYItemBase* SourceItem);

	// 쿨다운 적용
	void ApplyTrapCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo);
};