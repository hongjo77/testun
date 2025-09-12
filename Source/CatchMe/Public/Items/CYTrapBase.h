#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "CYTrapBase.generated.h"

class ACYPlayerCharacter;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class CATCHME_API ACYTrapBase : public ACYItemBase
{
    GENERATED_BODY()

public:
    ACYTrapBase();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    float TriggerRadius = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    float ArmingDelay = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    float TrapLifetime = 60.0f;

protected:
    virtual void BeginPlay() override;

    // 트랩 트리거 처리
    UFUNCTION()
    void OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // 트랩 활성화
    UFUNCTION()
    void ArmTrap();

    // 효과 적용
    void ApplyTrapEffectsToTarget(ACYPlayerCharacter* Target);
    void ApplySingleEffect(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass);

    // 타이머 설정
    void SetupTrapTimers();

private:
    bool bIsArmed = false;
    FTimerHandle ArmingTimer;
    FTimerHandle LifetimeTimer;
};