// CYSlowTrap.h - 개선된 헤더 파일
#pragma once

#include "CoreMinimal.h"
#include "Items/CYTrapBase.h"
#include "CYSlowTrap.generated.h"

/**
 * 슬로우 트랩 - 적의 이동속도를 감소시키는 트랩
 * 고유한 시각적 특성: 파란색 넓은 원판 모양, 약간 투명한 효과
 */
UCLASS(BlueprintType)
class CATCHME_API ACYSlowTrap : public ACYTrapBase
{
	GENERATED_BODY()

public:
	ACYSlowTrap();

	// 슬로우 트랩 전용 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slow Trap")
	float SlowPercentage = 0.5f; // 50% 속도 감소

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slow Trap")
	float SlowDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slow Trap")
	float SlowedMoveSpeed = 100.0f; // 슬로우 시 이동속도

protected:
	// ✅ 오버라이드된 이벤트 함수들
	virtual void OnTrapSpawned_Implementation() override;
	virtual void OnTrapArmed_Implementation() override;
	virtual void OnTrapTriggered_Implementation(ACYPlayerCharacter* Target) override;
    
	// ✅ 시각적/오디오 설정 오버라이드
	virtual void SetupTrapVisuals_Implementation() override;
	virtual void PlayTrapSound_Implementation() override;
    
	// ✅ 슬로우 트랩만의 커스텀 효과
	virtual void ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target) override;

private:
	// ✅ 슬로우 효과 관련 함수들
	void ApplySlowEffect(ACYPlayerCharacter* Target);
	void ShowSlowVisualEffect();
	
	// ✅ 슬로우 트랩 전용 트리거 효과
	void PlaySlowTriggerEffect();
};