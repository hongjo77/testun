// Weapon/CYWeaponBase.h
#pragma once

#include "CoreMinimal.h"
#include "Item/CYItemBase.h"
#include "Engine/TimerHandle.h"
#include "CYWeaponBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class CATCHME_API ACYWeaponBase : public ACYItemBase
{
	GENERATED_BODY()

public:
	ACYWeaponBase();

	// 무기 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float BaseDamage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float AttackCooldown = 1.0f;

	// 공격 함수
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerAttack(FVector HitLocation, AActor* HitTarget);

	// 공격 이펙트
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAttackEffect(FVector HitLocation);

	// 쿨다운 체크
	UFUNCTION(BlueprintCallable)
	bool CanAttack();

	// 남은 쿨다운 시간
	UFUNCTION(BlueprintCallable)
	float GetRemainingCooldown();

protected:
	// 마지막 공격 시간
	float LastAttackTime = 0.0f;

	// 무기별 공격 로직 (자식 클래스에서 구현)
	virtual void ExecuteAttack(FVector HitLocation, AActor* HitTarget);

	// 무기별 특수 효과 (자식 클래스에서 구현)
	virtual void ApplyWeaponEffect(AActor* HitTarget);

	// 데미지 적용
	UFUNCTION(BlueprintCallable)
	void ApplyDamage(AActor* Target, float Damage);
};