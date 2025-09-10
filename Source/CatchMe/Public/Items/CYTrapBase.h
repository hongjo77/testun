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

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void ArmTrap();

    bool bIsArmed = false;
    FTimerHandle ArmingTimer;
    FTimerHandle LifetimeTimer;
};