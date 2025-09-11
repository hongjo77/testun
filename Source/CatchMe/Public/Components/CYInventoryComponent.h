#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CYInventoryComponent.generated.h"

class ACYItemBase;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, int32, SlotIndex, ACYItemBase*, Item);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHME_API UCYInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYInventoryComponent();

	// ✅ 무기와 아이템 슬롯 분리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 WeaponSlotCount = 3;  // 무기 슬롯 개수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 ItemSlotCount = 10;   // 아이템 슬롯 개수

	// 무기 슬롯들 (1~3번 키)
	UPROPERTY(ReplicatedUsing = OnRep_WeaponSlots, BlueprintReadOnly, Category = "Inventory")
	TArray<ACYItemBase*> WeaponSlots;

	// 아이템 슬롯들 (4~9번 키)
	UPROPERTY(ReplicatedUsing = OnRep_ItemSlots, BlueprintReadOnly, Category = "Inventory")
	TArray<ACYItemBase*> ItemSlots;

	// 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryChanged OnInventoryChanged;

	// 인벤토리 관리
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 FindEmptySlot() const;

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

	// ✅ 새로운 함수들
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemWithStacking(ACYItemBase* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddWeapon(ACYItemBase* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void PrintInventoryStatus() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_WeaponSlots();

	UFUNCTION()
	void OnRep_ItemSlots();

	// 소유자의 ASC 가져오기
	UAbilitySystemComponent* GetOwnerASC() const;

	// ✅ 헬퍼 함수들
	int32 FindEmptyWeaponSlot() const;
	int32 FindEmptyItemSlot() const;
	int32 FindStackableItemSlot(ACYItemBase* Item) const;
};