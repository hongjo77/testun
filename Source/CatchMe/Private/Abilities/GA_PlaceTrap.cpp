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
    
    // 🔥 AbilityTags deprecated 경고 해결 - SetAssetTags 사용
    const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
    
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(GameplayTags.Ability_Trap_Place);
    SetAssetTags(AssetTags);
    
    // Activation Blocked Tags
    FGameplayTagContainer BlockedTags;
    BlockedTags.AddTag(GameplayTags.State_Stunned);
    BlockedTags.AddTag(GameplayTags.State_Dead);
    ActivationBlockedTags = BlockedTags;
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

    // 트랩 설치 위치 계산
    FHitResult HitResult;
    FVector SpawnLocation;
    
    UCYWeaponComponent* WeaponComp = OwnerActor->FindComponentByClass<UCYWeaponComponent>();
    if (WeaponComp && WeaponComp->PerformLineTrace(HitResult, 300.0f))
    {
        SpawnLocation = HitResult.Location;
        UE_LOG(LogTemp, Warning, TEXT("Trap spawn location (line trace): %s"), *SpawnLocation.ToString());
    }
    else
    {
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
        UE_LOG(LogTemp, Warning, TEXT("✅ SUCCESS: Trap spawned at %s by %s"), 
               *SpawnLocation.ToString(), *OwnerActor->GetName());

        // 🔍 소스 오브젝트 디버깅 - 더 자세히
        UE_LOG(LogTemp, Warning, TEXT("🔍 === SOURCE OBJECT DEBUGGING ==="));
        
        const FGameplayAbilitySpec* CurrentSpec = GetCurrentAbilitySpec();
        UE_LOG(LogTemp, Warning, TEXT("CurrentSpec: %s"), CurrentSpec ? TEXT("EXISTS") : TEXT("NULL"));
        
        if (CurrentSpec)
        {
            UE_LOG(LogTemp, Warning, TEXT("SourceObject.IsValid(): %s"), 
                   CurrentSpec->SourceObject.IsValid() ? TEXT("YES") : TEXT("NO"));
            
            if (CurrentSpec->SourceObject.IsValid())
            {
                UObject* SourceObj = CurrentSpec->SourceObject.Get();
                UE_LOG(LogTemp, Warning, TEXT("SourceObject: %s (Class: %s)"), 
                       *SourceObj->GetName(), *SourceObj->GetClass()->GetName());
                
                if (ACYItemBase* UsedItem = Cast<ACYItemBase>(SourceObj))
                {
                    UE_LOG(LogTemp, Warning, TEXT("✅ Cast to CYItemBase SUCCESS"));
                    UE_LOG(LogTemp, Warning, TEXT("ItemName: %s"), *UsedItem->ItemName.ToString());
                    UE_LOG(LogTemp, Warning, TEXT("DesiredTrapEffects.Num(): %d"), UsedItem->DesiredTrapEffects.Num());
                    
                    // 각 DesiredTrapEffect 로그
                    for (int32 i = 0; i < UsedItem->DesiredTrapEffects.Num(); i++)
                    {
                        if (UsedItem->DesiredTrapEffects[i])
                        {
                            UE_LOG(LogTemp, Warning, TEXT("  DesiredEffect[%d]: %s"), 
                                   i, *UsedItem->DesiredTrapEffects[i]->GetName());
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("  DesiredEffect[%d]: NULL"), i);
                        }
                    }
                    
                    if (UsedItem->DesiredTrapEffects.Num() > 0)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("🎯 COPYING DesiredTrapEffects to Trap"));
                        Trap->ItemEffects = UsedItem->DesiredTrapEffects;
                        UE_LOG(LogTemp, Warning, TEXT("✅ Applied %d custom effects to trap"), UsedItem->DesiredTrapEffects.Num());
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("❌ DesiredTrapEffects is EMPTY!"));
                        UE_LOG(LogTemp, Error, TEXT("❌ Check BP_TestTrap DesiredTrapEffects array!"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("❌ Cast to CYItemBase FAILED"));
                    UE_LOG(LogTemp, Error, TEXT("❌ SourceObject class: %s"), *SourceObj->GetClass()->GetName());
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ SourceObject is INVALID"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ GetCurrentAbilitySpec() returned NULL"));
            UE_LOG(LogTemp, Error, TEXT("❌ This means ability wasn't triggered by UseItem"));
        }
        
        // 🔍 최종 결과 확인
        UE_LOG(LogTemp, Warning, TEXT("🎯 FINAL: Trap->ItemEffects.Num() = %d"), Trap->ItemEffects.Num());
        UE_LOG(LogTemp, Warning, TEXT("🔍 === END SOURCE OBJECT DEBUGGING ==="));
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