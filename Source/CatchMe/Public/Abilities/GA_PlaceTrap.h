// GA_PlaceTrap.h - 개선된 헤더 파일
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Items/CYTrapData.h"
#include "GA_PlaceTrap.generated.h"

class ACYTrapBase;
class ACYItemBase;

/**
 * 트랩 배치 어빌리티 - 트랩별 고유 특성을 반영한 개선된 버전
 * 각 트랩 타입에 맞는 메쉬, 사운드, 이펙트를 자동으로 적용
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

	// ✅ 소스 아이템 관리 (블루프린트 노출 불가능한 타입이므로 UFUNCTION 제거)
	ACYItemBase* GetSourceItemFromAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo);

	// ✅ 트랩 생성 정보 로깅
	UFUNCTION(BlueprintCallable, Category = "Trap")
	void LogTrapCreationInfo(ACYItemBase* SourceItem);

	// ✅ 개선된 트랩 생성
	UFUNCTION(BlueprintCallable, Category = "Trap")
	ACYTrapBase* CreateTrapFromSourceItem(ACYItemBase* SourceItem, 
		const FVector& SpawnLocation, const FRotator& SpawnRotation, AActor* OwnerActor);

	// ✅ 새 트랩 설정
	UFUNCTION(BlueprintCallable, Category = "Trap")
	void ConfigureNewTrap(ACYTrapBase* NewTrap, ACYItemBase* SourceItem);

	// ✅ 트랩별 성공 메시지 표시
	UFUNCTION(BlueprintCallable, Category = "Trap")
	void ShowTrapPlacementSuccess(ACYTrapBase* NewTrap);

	// 트랩 배치 위치 계산
	UFUNCTION(BlueprintCallable, Category = "Trap")
	FVector CalculateSpawnLocation(AActor* OwnerActor);

	// ✅ 인벤토리에서 아이템 관리
	UFUNCTION(BlueprintCallable, Category = "Trap")
	ACYItemBase* FindValidTrapItemInInventory(AActor* OwnerActor);

	// ✅ 특정 아이템만 소모하는 함수
	UFUNCTION(BlueprintCallable, Category = "Trap")
	void ConsumeSpecificItemFromInventory(AActor* OwnerActor, ACYItemBase* SourceItem);

	// 쿨다운 적용
	void ApplyTrapCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo);
};