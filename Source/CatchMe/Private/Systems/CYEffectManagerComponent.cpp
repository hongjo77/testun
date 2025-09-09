// Systems/CYEffectManagerComponent.cpp
#include "Systems/CYEffectManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

UCYEffectManagerComponent::UCYEffectManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UCYEffectManagerComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCYEffectManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (GetOwner()->HasAuthority())
    {
        UpdateEffects(DeltaTime);
    }
}

void UCYEffectManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UCYEffectManagerComponent, ActiveEffects);
}

void UCYEffectManagerComponent::ServerApplyEffect_Implementation(FGameplayTag EffectType, float Duration, float Magnitude, AActor* Instigator)
{
    if (!GetOwner()->HasAuthority()) return;

    // 기존 같은 효과 제거
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i].EffectType == EffectType)
        {
            ActiveEffects.RemoveAt(i);
            break;
        }
    }

    // 새 효과 추가
    FCYActiveEffect NewEffect;
    NewEffect.EffectType = EffectType;
    NewEffect.RemainingTime = Duration;
    NewEffect.Magnitude = Magnitude;
    NewEffect.Instigator = Instigator;
    
    ActiveEffects.Add(NewEffect);

    // 이벤트 브로드캐스트
    OnEffectApplied.Broadcast(EffectType, Duration, Magnitude);
}

void UCYEffectManagerComponent::ServerRemoveEffect_Implementation(FGameplayTag EffectType)
{
    if (!GetOwner()->HasAuthority()) return;

    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i].EffectType == EffectType)
        {
            ActiveEffects.RemoveAt(i);
            OnEffectRemoved.Broadcast(EffectType);
            break;
        }
    }
}

void UCYEffectManagerComponent::ServerClearAllEffects_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

    for (const FCYActiveEffect& Effect : ActiveEffects)
    {
        OnEffectRemoved.Broadcast(Effect.EffectType);
    }
    
    ActiveEffects.Empty();
}

bool UCYEffectManagerComponent::HasEffect(FGameplayTag EffectType)
{
    for (const FCYActiveEffect& Effect : ActiveEffects)
    {
        if (Effect.EffectType == EffectType)
        {
            return true;
        }
    }
    return false;
}

void UCYEffectManagerComponent::UpdateEffects(float DeltaTime)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        ActiveEffects[i].RemainingTime -= DeltaTime;
        
        if (ActiveEffects[i].RemainingTime <= 0.0f)
        {
            RemoveExpiredEffect(i);
        }
    }
}

void UCYEffectManagerComponent::RemoveExpiredEffect(int32 Index)
{
    if (ActiveEffects.IsValidIndex(Index))
    {
        FGameplayTag EffectType = ActiveEffects[Index].EffectType;
        ActiveEffects.RemoveAt(Index);
        OnEffectRemoved.Broadcast(EffectType);
    }
}