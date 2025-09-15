#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "Items/CYTrapData.h"
#include "CYTrapBase.generated.h"

class ACYPlayerCharacter;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS(Abstract, BlueprintType)
class CATCHME_API ACYTrapBase : public ACYItemBase
{
    GENERATED_BODY()

public:
    ACYTrapBase();

    // 트랩 데이터 (데이터 테이블에서 가져올 수도 있음)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Data")
    FTrapData TrapData;

    // 트랩 타입 (하위 클래스에서 설정)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    ETrapType TrapType;

    // 트랩 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings")
    float TriggerRadius = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings")
    float ArmingDelay = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings")
    float TrapLifetime = 60.0f;

    // 현재 상태
    UPROPERTY(BlueprintReadOnly, Category = "Trap State")
    bool bIsArmed = false;

    // 가상 함수들 - 하위 클래스에서 구현
    UFUNCTION(BlueprintNativeEvent, Category = "Trap Events")
    void OnTrapSpawned();
    virtual void OnTrapSpawned_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Trap Events")
    void OnTrapArmed();
    virtual void OnTrapArmed_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Trap Events")
    void OnTrapTriggered(ACYPlayerCharacter* Target);
    virtual void OnTrapTriggered_Implementation(ACYPlayerCharacter* Target);

    UFUNCTION(BlueprintNativeEvent, Category = "Trap Events")
    void OnTrapDestroyed();
    virtual void OnTrapDestroyed_Implementation();

    // 트랩 효과 적용 (템플릿 메서드 패턴)
    UFUNCTION(BlueprintCallable, Category = "Trap")
    void ApplyTrapEffects(ACYPlayerCharacter* Target);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 트랩 시각적 설정 (하위 클래스에서 구현)
    UFUNCTION(BlueprintNativeEvent, Category = "Trap Visuals")
    void SetupTrapVisuals();
    virtual void SetupTrapVisuals_Implementation();

    // 트랩 사운드 재생 (하위 클래스에서 구현)
    UFUNCTION(BlueprintNativeEvent, Category = "Trap Audio")
    void PlayTrapSound();
    virtual void PlayTrapSound_Implementation();

    // 하위 클래스별 커스텀 효과 적용
    UFUNCTION(BlueprintNativeEvent, Category = "Trap Effects")
    void ApplyCustomEffects(ACYPlayerCharacter* Target);
    virtual void ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target);

    // 오버랩 이벤트 핸들러
    UFUNCTION()
    void OnTriggerSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // 타이머 핸들러
    UFUNCTION()
    void ArmTrap();

    // 개별 효과 적용
    void ApplySingleEffect(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass);

    // 타이머 설정
    void SetupTrapTimers();

private:
    FTimerHandle ArmingTimer;
    FTimerHandle LifetimeTimer;
};