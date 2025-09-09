// Systems/CYWeaponManagerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "CYWeaponManagerComponent.generated.h"

class ACYWeaponBase;

UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CATCHME_API UCYWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYWeaponManagerComponent();

	// 현재 장착된 무기
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	ACYWeaponBase* CurrentWeapon;

	// 무기 장착 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponSocketName = "hand_r_socket";

	// 무기 장착
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerEquipWeapon(ACYWeaponBase* Weapon);

	// 무기 사용 (공격)
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerUseWeapon(FVector HitLocation, AActor* HitTarget);

	// 무기 해제
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerUnequipWeapon();

	// 무기 장착 이벤트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, ACYWeaponBase*, Weapon);
	UPROPERTY(BlueprintAssignable)
	FOnWeaponEquipped OnWeaponEquipped;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponUnequipped);
	UPROPERTY(BlueprintAssignable)
	FOnWeaponUnequipped OnWeaponUnequipped;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 무기 물리적 장착
	void AttachWeaponToCharacter(ACYWeaponBase* Weapon);
	void DetachWeaponFromCharacter();
};