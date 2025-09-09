// Item/CYTrapBase.h
#pragma once

#include "CoreMinimal.h"
#include "Item/CYItemBase.h"
#include "Components/SphereComponent.h"
#include "Engine/TimerHandle.h"
#include "CYTrapBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class CATCHME_API ACYTrapBase : public ACYItemBase
{
    GENERATED_BODY()

public:
    ACYTrapBase();

    // 트랩 활성화 범위
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
    USphereComponent* TriggerCollision;

    // 트랩 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Settings")
    float TrapLifetime = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Settings")
    bool bSingleUse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Settings")
    float ArmingDelay = 2.0f;

    // 트랩 상태
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trap State")
    bool bTriggered = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trap State")
    bool bArmed = false;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 트랩 발동
    UFUNCTION()
    void OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                        bool bFromSweep, const FHitResult& SweepResult);

    // 트랩 활성화
    UFUNCTION()
    void ArmTrap();

    // 트랩 효과 적용 (자식 클래스에서 오버라이드)
    virtual void ApplyTrapEffect(AActor* Target);

    // 트랩 발동 조건 체크
    virtual bool CanTrigger(AActor* Target);

    // 타이머 핸들들
    UPROPERTY()
    FTimerHandle LifetimeTimer;

    UPROPERTY()
    FTimerHandle ArmingTimer;

    UPROPERTY()
    FTimerHandle RemovalTimer;

    UFUNCTION()
    void RemoveTrap();

    // 트랩 이펙트
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayTrapEffect();
};