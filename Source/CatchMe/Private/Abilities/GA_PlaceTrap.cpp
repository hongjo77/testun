#include "Abilities/GA_PlaceTrap.h"
#include "Player/CYPlayerCharacter.h"
#include "Items/CYTrapBase.h"
#include "Engine/World.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "GAS/CYGameplayEffects.h"

UGA_PlaceTrap::UGA_PlaceTrap()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    TrapClass = ACYTrapBase::StaticClass();
    
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Trap.Place"));
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Stunned"));
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
}

void UGA_PlaceTrap::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Warning, TEXT("UGA_PlaceTrap::ActivateAbility called"));
    
    if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaceTrap: No authority or prediction key - ending ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("PlaceTrap: Authority check passed"));

    ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetAvatarActorFromActorInfo());
    
    // 디버깅 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: %s"), PlayerCharacter ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("TrapClass: %s"), TrapClass ? TEXT("Valid") : TEXT("NULL"));
    
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaceTrap: PlayerCharacter is NULL"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    if (!TrapClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaceTrap: TrapClass is NULL - Need to set in Blueprint!"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 쿨다운 체크
    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaceTrap: On cooldown"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 트랩 설치 위치 계산
    FHitResult HitResult;
    FVector SpawnLocation;
    
    if (PlayerCharacter->PerformLineTrace(HitResult, 300.0f))
    {
        SpawnLocation = HitResult.Location;
        UE_LOG(LogTemp, Warning, TEXT("Trap spawn location (line trace): %s"), *SpawnLocation.ToString());
    }
    else
    {
        SpawnLocation = PlayerCharacter->GetActorLocation() + 
                    PlayerCharacter->GetActorForwardVector() * 200.0f;
        SpawnLocation.Z = PlayerCharacter->GetActorLocation().Z;
        UE_LOG(LogTemp, Warning, TEXT("Trap spawn location (forward): %s"), *SpawnLocation.ToString());
    }

    FRotator SpawnRotation = PlayerCharacter->GetActorRotation();

    // 트랩 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = PlayerCharacter;
    SpawnParams.Instigator = PlayerCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    UE_LOG(LogTemp, Warning, TEXT("Attempting to spawn trap..."));
    
    if (ACYTrapBase* Trap = GetWorld()->SpawnActor<ACYTrapBase>(TrapClass, SpawnLocation, SpawnRotation, SpawnParams))
    {
        UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Trap placed at %s by %s"), 
           *SpawnLocation.ToString(), *PlayerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED: Could not spawn trap actor"));
    }

    // 쿨다운 적용
    FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(UGE_TrapPlaceCooldown::StaticClass(), 1);
    if (CooldownSpec.IsValid())
    {
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}