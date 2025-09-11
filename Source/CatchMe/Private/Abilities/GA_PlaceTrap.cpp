#include "Abilities/GA_PlaceTrap.h"
#include "Player/CYPlayerCharacter.h"
#include "Items/CYTrapBase.h"
#include "Engine/World.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "GAS/CYGameplayEffects.h"

UGA_PlaceTrap::UGA_PlaceTrap()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    
	// ✅ UE 5.6에서 실제 작동하는 방법 - 기존 방식 유지 (warning만 발생)
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Trap.Place"));
	
	// 블록 태그들 - 기존 방식 유지
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Stunned"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
}

void UGA_PlaceTrap::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
       EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
       return;
    }

    ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!PlayerCharacter || !TrapClass)
    {
       EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
       return;
    }

    // ✅ 쿨다운 체크 - C++ 클래스 직접 사용
    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 트랩 설치 위치 계산
    FHitResult HitResult;
    FVector SpawnLocation;
    
    if (PlayerCharacter->PerformLineTrace(HitResult, 300.0f))
    {
       SpawnLocation = HitResult.Location;
    }
    else
    {
       SpawnLocation = PlayerCharacter->GetActorLocation() + 
                   PlayerCharacter->GetActorForwardVector() * 200.0f;
       SpawnLocation.Z = PlayerCharacter->GetActorLocation().Z;
    }

    FRotator SpawnRotation = PlayerCharacter->GetActorRotation();

    // 트랩 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = PlayerCharacter;
    SpawnParams.Instigator = PlayerCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (ACYTrapBase* Trap = GetWorld()->SpawnActor<ACYTrapBase>(TrapClass, SpawnLocation, SpawnRotation, SpawnParams))
    {
       // ✅ C++로 만든 GameplayEffect 클래스 직접 사용
       Trap->TrapEffectClass = UGE_ImmobilizeTrap::StaticClass();
        
       UE_LOG(LogTemp, Warning, TEXT("Trap placed at %s by %s"), 
          *SpawnLocation.ToString(), *PlayerCharacter->GetName());
    }
    else
    {
       UE_LOG(LogTemp, Warning, TEXT("Failed to place trap"));
    }

    // ✅ 쿨다운 적용 - C++ 클래스 직접 사용
    FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(UGE_TrapPlaceCooldown::StaticClass(), 1);
    if (CooldownSpec.IsValid())
    {
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}