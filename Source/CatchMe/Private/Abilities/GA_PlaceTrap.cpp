#include "Abilities/GA_PlaceTrap.h"
#include "Items/CYTrapBase.h"
#include "Engine/World.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "GAS/CYGameplayEffects.h"
#include "CYGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Components/CYWeaponComponent.h"

UGA_PlaceTrap::UGA_PlaceTrap()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    TrapClass = ACYTrapBase::StaticClass();
    
    // ✅ 중앙집중식 태그 사용
    const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
    
    AbilityTags.AddTag(GameplayTags.Ability_Trap_Place);
    ActivationBlockedTags.AddTag(GameplayTags.State_Stunned);
    ActivationBlockedTags.AddTag(GameplayTags.State_Dead);
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

    // ✅ PlayerCharacter 대신 일반 Actor 사용
    AActor* OwnerActor = GetAvatarActorFromActorInfo();
    
    UE_LOG(LogTemp, Warning, TEXT("OwnerActor: %s"), OwnerActor ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("TrapClass: %s"), TrapClass ? TEXT("Valid") : TEXT("NULL"));
    
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaceTrap: OwnerActor is NULL"));
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

    // ✅ 트랩 설치 위치 계산 - WeaponComponent 사용 또는 직접 라인트레이스
    FHitResult HitResult;
    FVector SpawnLocation;
    
    // WeaponComponent를 찾아서 라인트레이스 시도
    UCYWeaponComponent* WeaponComp = OwnerActor->FindComponentByClass<UCYWeaponComponent>();
    if (WeaponComp && WeaponComp->PerformLineTrace(HitResult, 300.0f))
    {
        SpawnLocation = HitResult.Location;
        UE_LOG(LogTemp, Warning, TEXT("Trap spawn location (line trace): %s"), *SpawnLocation.ToString());
    }
    else
    {
        // 라인트레이스 실패시 앞쪽에 배치
        SpawnLocation = OwnerActor->GetActorLocation() + 
                    OwnerActor->GetActorForwardVector() * 200.0f;
        SpawnLocation.Z = OwnerActor->GetActorLocation().Z;
        UE_LOG(LogTemp, Warning, TEXT("Trap spawn location (forward): %s"), *SpawnLocation.ToString());
    }

    FRotator SpawnRotation = OwnerActor->GetActorRotation();

    // 트랩 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerActor;
    SpawnParams.Instigator = Cast<APawn>(OwnerActor);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    UE_LOG(LogTemp, Warning, TEXT("Attempting to spawn trap..."));
    
    if (ACYTrapBase* Trap = GetWorld()->SpawnActor<ACYTrapBase>(TrapClass, SpawnLocation, SpawnRotation, SpawnParams))
    {
        // 소스 오브젝트에서 원하는 효과 가져오기
        if (const FGameplayAbilitySpec* CurrentSpec = GetCurrentAbilitySpec())
        {
            if (CurrentSpec->SourceObject.IsValid())
            {
                if (ACYItemBase* UsedItem = Cast<ACYItemBase>(CurrentSpec->SourceObject.Get()))
                {
                    if (UsedItem->DesiredTrapEffects.Num() > 0)
                    {
                        Trap->ItemEffects = UsedItem->DesiredTrapEffects;
                        UE_LOG(LogTemp, Warning, TEXT("Applied %d custom effects to trap"), UsedItem->DesiredTrapEffects.Num());
                    }
                }
            }
        }
    
        UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Trap placed at %s by %s"), 
               *SpawnLocation.ToString(), *OwnerActor->GetName());
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