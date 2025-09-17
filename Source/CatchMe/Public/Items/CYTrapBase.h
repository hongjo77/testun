// CYTrapBase.h - 시각적 업데이트 멀티캐스트 함수 추가
#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "Items/CYTrapData.h"
#include "CYTrapBase.generated.h"

class ACYPlayerCharacter;
class UAbilitySystemComponent;
class UGameplayEffect;

UENUM(BlueprintType)
enum class ETrapState : uint8
{
    MapPlaced       UMETA(DisplayName = "Map Placed (Pickupable)"),
    PlayerPlaced    UMETA(DisplayName = "Player Placed (Active)")
};

UCLASS(Abstract, BlueprintType)
class CATCHME_API ACYTrapBase : public ACYItemBase
{
    GENERATED_BODY()

public:
    ACYTrapBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap State", Replicated)
    ETrapState TrapState = ETrapState::MapPlaced;

    UPROPERTY(BlueprintReadOnly, Category = "Trap State", Replicated)
    bool bIsArmed = false;

    UFUNCTION(BlueprintCallable, Category = "Trap")
    void ConvertToPlayerPlacedTrap(AActor* PlacingPlayer);

    // ✅ 리플리케이션 추가
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Data", Replicated)
    FTrapData TrapData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap", Replicated)
    ETrapType TrapType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings")
    float TriggerRadius = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings")
    float ArmingDelay = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings")
    float TrapLifetime = 60.0f;

    // 가상 함수들
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

    // ✅ 멀티캐스트 함수들
    UFUNCTION(NetMulticast, Reliable, Category = "Trap Events")
    void MulticastOnTrapTriggered(ACYPlayerCharacter* Target);

    // ✅ 새로운 시각적 업데이트 멀티캐스트 함수
    UFUNCTION(NetMulticast, Reliable, Category = "Trap Visuals")
    void MulticastUpdateTrapVisuals();

    UFUNCTION(BlueprintCallable, Category = "Trap")
    void ApplyTrapEffects(ACYPlayerCharacter* Target);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 트랩 시각적 설정
    UFUNCTION(BlueprintNativeEvent, Category = "Trap Visuals")
    void SetupTrapVisuals();
    virtual void SetupTrapVisuals_Implementation();

    // 트랩 사운드 재생
    UFUNCTION(BlueprintNativeEvent, Category = "Trap Audio")
    void PlayTrapSound();
    virtual void PlayTrapSound_Implementation();

    // 하위 클래스별 커스텀 효과 적용
    UFUNCTION(BlueprintNativeEvent, Category = "Trap Effects")
    void ApplyCustomEffects(ACYPlayerCharacter* Target);
    virtual void ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target);

    // 오버랩 이벤트 핸들러들
    UFUNCTION()
    void OnTriggerSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPickupSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPickupSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 타이머 핸들러
    UFUNCTION()
    void ArmTrap();

    // 트랩 상태 설정
    void SetupTrapForCurrentState();

    // 개별 효과 적용
    void ApplySingleEffect(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass);

    // 타이머 설정
    void SetupTrapTimers();

private:
    FTimerHandle ArmingTimer;
    FTimerHandle LifetimeTimer;
};