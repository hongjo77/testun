#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "CYTrapBase.generated.h"

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

    UPROPERTY(BlueprintReadWrite, Category = "GAS")
    TSubclassOf<class UGameplayEffect> TrapEffectClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Movement")
    bool bUseCustomMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Movement", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float CustomMoveSpeed = 0.0f; // 실제 속도값 (0 = 정지, 600 = 정상속도)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Movement")
    float CustomDuration = 5.0f;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void ArmTrap();

    void ApplyCustomMovementEffect(class UAbilitySystemComponent* TargetASC);

    bool bIsArmed = false;
    FTimerHandle ArmingTimer;
    FTimerHandle LifetimeTimer;
};