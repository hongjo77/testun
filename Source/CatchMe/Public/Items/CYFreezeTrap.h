#pragma once

#include "CoreMinimal.h"
#include "Items/CYTrapBase.h"
#include "CYFreezeTrap.generated.h"

/**
 * 프리즈 트랩 - 적을 완전히 정지시키는 트랩
 */
UCLASS(BlueprintType)
class CATCHME_API ACYFreezeTrap : public ACYTrapBase
{
	GENERATED_BODY()

public:
	ACYFreezeTrap();

	// 프리즈 트랩 전용 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Freeze Trap")
	float FreezeDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Freeze Trap")
	bool bDisableJumping = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Freeze Trap")
	bool bDisableAbilities = true;

protected:
	// 오버라이드된 이벤트 함수들
	virtual void OnTrapSpawned_Implementation() override;
	virtual void OnTrapArmed_Implementation() override;
	virtual void OnTrapTriggered_Implementation(ACYPlayerCharacter* Target) override;
    
	// 시각적/오디오 설정
	virtual void SetupTrapVisuals_Implementation() override;
	virtual void PlayTrapSound_Implementation() override;
    
	// 프리즈 트랩만의 커스텀 효과
	virtual void ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target) override;

private:
	// 프리즈 효과 관련 함수들
	void ApplyFreezeEffect(ACYPlayerCharacter* Target);
	void ShowFreezeVisualEffect();
	void CreateIceEffect();
};