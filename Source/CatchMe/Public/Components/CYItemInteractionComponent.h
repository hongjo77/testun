#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CYItemInteractionComponent.generated.h"

class ACYItemBase;
class UCYInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNearbyItemChanged, ACYItemBase*, Item, bool, bNear);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CATCHME_API UCYItemInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYItemInteractionComponent();

	// 상호작용 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 200.0f;

	// ✅ 체크 주기 (성능 개선)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float CheckInterval = 0.1f; // 0.1초마다 체크

	// 현재 근처 아이템
	UPROPERTY(ReplicatedUsing = OnRep_NearbyItem, BlueprintReadOnly, Category = "Interaction")
	ACYItemBase* NearbyItem;

	// 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNearbyItemChanged OnNearbyItemChanged;

	// 상호작용 함수들
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractWithNearbyItem();

	UFUNCTION(Server, Reliable, Category = "Interaction")
	void ServerPickupItem(ACYItemBase* Item);

protected:
	virtual void BeginPlay() override;
	// ✅ Tick 제거
	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_NearbyItem();

	// ✅ 타이머 기반으로 변경
	UFUNCTION()
	void CheckForNearbyItems();
    
	UCYInventoryComponent* GetInventoryComponent() const;

private:
	// ✅ 타이머 핸들 추가
	FTimerHandle ItemCheckTimer;
};