// CYInventoryComponent.h - 중복 방지 플래그 추가
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "CYInventoryComponent.generated.h"

class ACYItemBase;
class ACYWeaponBase;
class UAbilitySystemComponent;
class UCYAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, int32, SlotIndex, ACYItemBase*, Item);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHME_API UCYInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYInventoryComponent();

	// 슬롯 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 WeaponSlotCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 ItemSlotCount = 10;

	// 슬롯 배열
	UPROPERTY(ReplicatedUsing = OnRep_WeaponSlots, BlueprintReadOnly, Category = "Inventory")
	TArray<ACYItemBase*> WeaponSlots;

	UPROPERTY(ReplicatedUsing = OnRep_ItemSlots, BlueprintReadOnly, Category = "Inventory")
	TArray<ACYItemBase*> ItemSlots;

	// 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryChanged OnInventoryChanged;

	// 공개 인터페이스
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(ACYItemBase* Item, int32 SlotIndex = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	ACYItemBase* GetItem(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 SlotIndex);

	UFUNCTION(Server, Reliable, Category = "Inventory")
	void ServerUseItem(int32 SlotIndex);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ✅ 중복 사용 방지 플래그
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	bool bIsProcessingUse = false;

	// 리플리케이션 응답
	UFUNCTION()
	void OnRep_WeaponSlots();

	UFUNCTION()
	void OnRep_ItemSlots();

	// 핵심 로직
	bool AddWeapon(ACYItemBase* Weapon);
	bool AddItemWithStacking(ACYItemBase* Item);

	// 유틸리티 함수
	int32 FindEmptyWeaponSlot() const;
	int32 FindEmptyItemSlot() const;
	int32 FindStackableItemSlot(ACYItemBase* Item) const;
	bool TryStackWithExistingItem(ACYItemBase* Item);
	void AutoEquipFirstWeapon(ACYWeaponBase* Weapon);

	// 아이템 사용 관련
	bool EquipWeaponFromSlot(ACYItemBase* Item);
	bool ActivateItemAbility(ACYItemBase* Item, int32 SlotIndex);
	
	// ✅ 트랩 아이템 전용 사용 로직 - 직접 실행
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseTrapItemDirect(ACYItemBase* Item, int32 LocalIndex);
	
	void ProcessItemConsumption(ACYItemBase* Item, int32 SlotIndex);

	// 제거 관련
	bool RemoveWeaponFromSlot(int32 WeaponIndex);
	bool RemoveItemFromSlot(int32 ItemIndex);

	// ASC 접근
	UAbilitySystemComponent* GetOwnerASC() const;
};