#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CYWeaponComponent.generated.h"

class ACYWeaponBase;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChanged, ACYWeaponBase*, OldWeapon, ACYWeaponBase*, NewWeapon);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHME_API UCYWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYWeaponComponent();

	// 현재 장착된 무기
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Weapon")
	ACYWeaponBase* CurrentWeapon;

	// 무기 장착 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponSocketName = TEXT("hand_r");

	// 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeaponChanged OnWeaponChanged;

	// 무기 관리
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeapon(ACYWeaponBase* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool UnequipWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool PerformAttack();

	// 라인 트레이스 (무기 공격용)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool PerformLineTrace(FHitResult& OutHit, float Range = 1000.0f);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UAbilitySystemComponent* GetOwnerASC() const;
	USkeletalMeshComponent* GetOwnerMesh() const;
};