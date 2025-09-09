// Systems/EffectManagerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "CYEffectManagerComponent.generated.h"

USTRUCT(BlueprintType)
struct CATCHME_API FCYActiveEffect
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FGameplayTag EffectType;

    UPROPERTY(BlueprintReadOnly)
    float RemainingTime;

    UPROPERTY(BlueprintReadOnly)
    float Magnitude;

    UPROPERTY(BlueprintReadOnly)
    AActor* Instigator;

    FCYActiveEffect()
    {
        RemainingTime = 0.0f;
        Magnitude = 1.0f;
        Instigator = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CATCHME_API UCYEffectManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCYEffectManagerComponent();

    // 현재 활성화된 효과들
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Effects")
    TArray<FCYActiveEffect> ActiveEffects;

    // 효과 적용 (서버)
    UFUNCTION(Server, Reliable, BlueprintCallable)
    void ServerApplyEffect(FGameplayTag EffectType, float Duration, float Magnitude, AActor* Instigator = nullptr);

    // 효과 제거
    UFUNCTION(Server, Reliable, BlueprintCallable) 
    void ServerRemoveEffect(FGameplayTag EffectType);

    // 특정 효과 보유 체크
    UFUNCTION(BlueprintCallable)
    bool HasEffect(FGameplayTag EffectType);

    // 모든 효과 제거
    UFUNCTION(Server, Reliable, BlueprintCallable)
    void ServerClearAllEffects();

    // 효과 적용 이벤트
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEffectApplied, FGameplayTag, EffectType, float, Duration, float, Magnitude);
    UPROPERTY(BlueprintAssignable)
    FOnEffectApplied OnEffectApplied;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectRemoved, FGameplayTag, EffectType);
    UPROPERTY(BlueprintAssignable) 
    FOnEffectRemoved OnEffectRemoved;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    void UpdateEffects(float DeltaTime);
    void RemoveExpiredEffect(int32 Index);
};