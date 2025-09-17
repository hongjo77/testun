// CYTrapBase.h - 멀티캐스트 함수 선언 추가
#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "Items/CYTrapData.h"
#include "CYTrapBase.generated.h"

class ACYPlayerCharacter;
class UAbilitySystemComponent;
class UGameplayEffect;

// ✅ 트랩 상태 enum 추가
UENUM(BlueprintType)
enum class ETrapState : uint8
{
    MapPlaced       UMETA(DisplayName = "Map Placed (Pickupable)"),    // 맵에 배치된 상태 (픽업 가능)
    PlayerPlaced    UMETA(DisplayName = "Player Placed (Active)")      // 플레이어가 설치한 상태 (활성화됨)
};

UCLASS(Abstract, BlueprintType)
class CATCHME_API ACYTrapBase : public ACYItemBase
{
    GENERATED_BODY()

public:
    ACYTrapBase();

    // ✅ 트랩 상태 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap State", Replicated)
    ETrapState TrapState = ETrapState::MapPlaced;

    // ✅ Armed 상태도 리플리케이트
    UPROPERTY(BlueprintReadOnly, Category = "Trap State", Replicated)
    bool bIsArmed = false;

    // ✅ 플레이어가 설치한 트랩으로 전환하는 함수
    UFUNCTION(BlueprintCallable, Category = "Trap")
    void ConvertToPlayerPlacedTrap(AActor* PlacingPlayer);

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

    // ✅ 새로운 멀티캐스트 함수 - 클라이언트 동기화용
    UFUNCTION(NetMulticast, Reliable, Category = "Trap Events")
    void MulticastOnTrapTriggered(ACYPlayerCharacter* Target);

    // 트랩 효과 적용 (템플릿 메서드 패턴)
    UFUNCTION(BlueprintCallable, Category = "Trap")
    void ApplyTrapEffects(ACYPlayerCharacter* Target);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

    // ✅ 상태별 오버랩 이벤트 핸들러
    UFUNCTION()
    void OnTriggerSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // ✅ 픽업용 오버랩 핸들러 (부모 함수 호출용)
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

    // ✅ 트랩 상태 설정
    void SetupTrapForCurrentState();

    // 개별 효과 적용
    void ApplySingleEffect(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass);

    // 타이머 설정
    void SetupTrapTimers();

private:
    FTimerHandle ArmingTimer;
    FTimerHandle LifetimeTimer;
};