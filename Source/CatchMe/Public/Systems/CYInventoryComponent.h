// Systems/CYInventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "CYInventoryComponent.generated.h"

class ACYItemBase;

UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CATCHME_API UCYInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYInventoryComponent();

	// 아이템 슬롯들
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	TArray<ACYItemBase*> Items;

	// 최대 슬롯 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxSlots = 6;

	// 아이템 추가
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerAddItem(ACYItemBase* Item);

	// 아이템 제거
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRemoveItem(int32 SlotIndex);

	// 특정 슬롯 아이템 가져오기
	UFUNCTION(BlueprintCallable)
	ACYItemBase* GetItemAtSlot(int32 SlotIndex);

	// 빈 슬롯 있는지 체크
	UFUNCTION(BlueprintCallable)
	bool HasEmptySlot();

	// 빈 슬롯 인덱스 찾기
	UFUNCTION(BlueprintCallable)
	int32 GetFirstEmptySlotIndex();

	// 특정 아이템 타입 개수 세기
	UFUNCTION(BlueprintCallable)
	int32 GetItemCount(FGameplayTag ItemType);

	// 인벤토리 업데이트 이벤트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
	UPROPERTY(BlueprintAssignable)
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	int32 CurrentSelectedSlot = 0;

	// 슬롯 선택
	UFUNCTION(BlueprintCallable)
	void SelectSlot(int32 SlotIndex);

	// 현재 선택된 아이템 가져오기
	UFUNCTION(BlueprintCallable)
	ACYItemBase* GetSelectedItem();

	// 선택된 아이템 사용
	UFUNCTION(BlueprintCallable)
	void UseSelectedItem();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_Items();
};