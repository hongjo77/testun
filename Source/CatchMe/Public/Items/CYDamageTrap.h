// CYDamageTrap.h - 개선된 헤더 파일
#pragma once

#include "CoreMinimal.h"
#include "Items/CYTrapBase.h"
#include "CYDamageTrap.generated.h"

/**
 * 데미지 트랩 - 적에게 직접적인 피해를 입히는 트랩
 * 고유한 시각적 특성: 빨간색 뾰족한 원뿔 모양, 강한 발광 효과
 */
UCLASS(BlueprintType)
class CATCHME_API ACYDamageTrap : public ACYTrapBase
{
	GENERATED_BODY()

public:
	ACYDamageTrap();

	// 데미지 트랩 전용 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Trap")
	float DamageAmount = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Trap")
	bool bInstantDamage = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Trap")
	float DamageOverTimeInterval = 1.0f; // DoT용

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Trap")
	int32 DamageOverTimeTicks = 3; // DoT 틱 수

protected:
	// ✅ 오버라이드된 이벤트 함수들
	virtual void OnTrapSpawned_Implementation() override;
	virtual void OnTrapArmed_Implementation() override;
	virtual void OnTrapTriggered_Implementation(ACYPlayerCharacter* Target) override;
    
	// ✅ 시각적/오디오 설정 오버라이드
	virtual void SetupTrapVisuals_Implementation() override;
	virtual void PlayTrapSound_Implementation() override;
    
	// ✅ 데미지 트랩만의 커스텀 효과
	virtual void ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target) override;

private:
	// ✅ 데미지 효과 관련 함수들
	void ApplyInstantDamage(ACYPlayerCharacter* Target);
	void ApplyDamageOverTime(ACYPlayerCharacter* Target);
	void ShowDamageVisualEffect();
	void CreateSpikeEffect();
	
	// ✅ 데미지 트랩 전용 트리거 효과
	void PlayDamageTriggerEffect();
};