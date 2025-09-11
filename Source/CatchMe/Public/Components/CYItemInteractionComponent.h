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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_NearbyItem();

	void CheckForNearbyItems();
	UCYInventoryComponent* GetInventoryComponent() const;
};