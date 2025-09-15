#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CYWeaponComponent.generated.h"

class ACYWeaponBase;
class UCYAbilitySystemComponent;
class USkeletalMeshComponent;

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

	// ✅ 클라이언트에서 직접 호출 가능한 인벤토리 표시
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DisplayInventoryStatus();

	// 클라이언트 RPC로 인벤토리 상태 표시 (백업용)
	UFUNCTION(Client, Reliable, Category = "Weapon")
	void ClientDisplayInventoryStatus();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentWeapon();

	// 핵심 로직 분리
	bool ExecuteWeaponAttack();

	// 헬퍼 함수들
	UCYAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;
	USkeletalMeshComponent* GetOwnerMesh() const;
	void AttachWeaponToOwner(ACYWeaponBase* Weapon);
	void DisableWeaponInteraction(ACYWeaponBase* Weapon);
};