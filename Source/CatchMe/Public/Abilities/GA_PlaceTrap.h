// GA_PlaceTrap.h - 특정 아이템 소모 함수 추가
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlaceTrap.generated.h"

class ACYTrapBase;
class ACYItemBase;

/**
 * 트랩 배치 어빌리티 - 사용자가 선택한 특정 아이템 사용
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

	// ✅ 인벤토리에서 아이템 관리
	UFUNCTION(BlueprintCallable, Category = "Trap")
	ACYItemBase* FindValidTrapItemInInventory(AActor* OwnerActor);

	// ✅ 특정 아이템만 소모하는 함수 (새로 추가)
	UFUNCTION(BlueprintCallable, Category = "Trap")
	void ConsumeSpecificItemFromInventory(AActor* OwnerActor, ACYItemBase* SourceItem);

	// 쿨다운 적용
	void ApplyTrapCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo);
};