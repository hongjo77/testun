// Systems/CYTrapManagerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "CYTrapManagerComponent.generated.h"

class ACYTrapBase;

UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CATCHME_API UCYTrapManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCYTrapManagerComponent();

	// 설치된 트랩들
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trap")
	TArray<ACYTrapBase*> PlacedTraps;

	// 최대 트랩 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	int32 MaxTraps = 3;

	// 트랩 설치
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerPlaceTrap(TSubclassOf<ACYTrapBase> TrapClass, FVector Location, FRotator Rotation);

	// 트랩 제거 (가장 오래된 것)
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRemoveOldestTrap();

	// 특정 트랩 제거
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRemoveTrap(ACYTrapBase* Trap);

	// 모든 트랩 정리
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerClearAllTraps();

	// 트랩 설치 이벤트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrapPlaced, ACYTrapBase*, Trap);
	UPROPERTY(BlueprintAssignable)
	FOnTrapPlaced OnTrapPlaced;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrapRemoved, ACYTrapBase*, Trap);
	UPROPERTY(BlueprintAssignable)
	FOnTrapRemoved OnTrapRemoved;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 트랩 정리 (널 포인터 제거)
	void CleanupTraps();
};