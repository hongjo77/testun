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

	// 인벤토리 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 InventorySize = 10;

	// 인벤토리 슬롯들
	UPROPERTY(ReplicatedUsing = OnRep_Inventory, BlueprintReadOnly, Category = "Inventory")
	TArray<ACYItemBase*> InventorySlots;

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

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Inventory();

	// 소유자의 ASC 가져오기
	UAbilitySystemComponent* GetOwnerASC() const;
};