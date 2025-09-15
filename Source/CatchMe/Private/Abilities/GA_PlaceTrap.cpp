#include "Abilities/GA_PlaceTrap.h"
#include "Items/CYTrapBase.h"
#include "Items/CYItemBase.h"
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
    
    const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
    
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(GameplayTags.Ability_Trap_Place);
    SetAssetTags(AssetTags);
    
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
    if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    AActor* OwnerActor = GetAvatarActorFromActorInfo();
    if (!OwnerActor || !TrapClass)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 쿨다운 체크
    const FGameplayTagContainer* CooldownTags = GetCooldownTags();
    if (CooldownTags && ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CooldownTags))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 트랩 설치 위치 계산
    FVector SpawnLocation = CalculateSpawnLocation(OwnerActor);
    FRotator SpawnRotation = OwnerActor->GetActorRotation();

    // 트랩 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerActor;
    SpawnParams.Instigator = Cast<APawn>(OwnerActor);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    if (ACYTrapBase* Trap = GetWorld()->SpawnActor<ACYTrapBase>(TrapClass, SpawnLocation, SpawnRotation, SpawnParams))
    {
        // ✅ 트랩 생성 후 즉시 커스텀 효과 설정 (TriggerEventData 전달)
        ConfigureTrapEffects(Trap, TriggerEventData);
        UE_LOG(LogTemp, Log, TEXT("Trap placed with %d effects"), Trap->ItemEffects.Num());
    }

    // 쿨다운 적용
    ApplyCooldown(Handle, ActorInfo, ActivationInfo);
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

FVector UGA_PlaceTrap::CalculateSpawnLocation(AActor* OwnerActor)
{
    FHitResult HitResult;
    UCYWeaponComponent* WeaponComp = OwnerActor->FindComponentByClass<UCYWeaponComponent>();
    
    if (WeaponComp && WeaponComp->PerformLineTrace(HitResult, 300.0f))
    {
        return HitResult.Location;
    }

    FVector ForwardLocation = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 200.0f;
    ForwardLocation.Z = OwnerActor->GetActorLocation().Z;
    return ForwardLocation;
}

void UGA_PlaceTrap::ConfigureTrapEffects(ACYTrapBase* Trap, const FGameplayEventData* TriggerEventData)
{
    if (!Trap) return;

    UE_LOG(LogTemp, Warning, TEXT("🔧 ConfigureTrapEffects: Starting trap effect configuration"));

    // ✅ 기본값 설정
    Trap->ItemEffects.Empty();
    Trap->ItemEffects.Add(UGE_ImmobilizeTrap::StaticClass());

    // ✅ 방법 1: GameplayEventData에서 아이템 정보 가져오기 (우선)
    if (TriggerEventData && TriggerEventData->OptionalObject)
    {
        const UObject* OptionalObj = TriggerEventData->OptionalObject;
        if (OptionalObj)
        {
            if (const ACYItemBase* UsedItem = Cast<ACYItemBase>(OptionalObj))
            {
                UE_LOG(LogTemp, Warning, TEXT("✅ Found item from EventData: %s with %d DesiredTrapEffects"), 
                       *UsedItem->ItemName.ToString(), UsedItem->DesiredTrapEffects.Num());
                
                if (UsedItem->DesiredTrapEffects.Num() > 0)
                {
                    Trap->ItemEffects = UsedItem->DesiredTrapEffects;
                    UE_LOG(LogTemp, Warning, TEXT("🎯 Trap configured with %d CUSTOM effects from EventData"), 
                           UsedItem->DesiredTrapEffects.Num());
                    
                    // 각 효과 클래스 이름 로그
                    for (int32 i = 0; i < UsedItem->DesiredTrapEffects.Num(); i++)
                    {
                        if (UsedItem->DesiredTrapEffects[i])
                        {
                            UE_LOG(LogTemp, Warning, TEXT("  🔥 Effect[%d]: %s"), i, *UsedItem->DesiredTrapEffects[i]->GetName());
                        }
                    }
                    return;
                }
            }
        }
    }

    // ✅ 방법 2: CurrentSpec의 SourceObject에서 아이템 정보 가져오기 (백업)
    const FGameplayAbilitySpec* CurrentSpec = GetCurrentAbilitySpec();
    if (CurrentSpec && CurrentSpec->SourceObject.IsValid())
    {
        UObject* SourceObjectPtr = CurrentSpec->SourceObject.Get();
        if (SourceObjectPtr)
        {
            if (const ACYItemBase* UsedItem = Cast<ACYItemBase>(SourceObjectPtr))
            {
                UE_LOG(LogTemp, Warning, TEXT("✅ Found item from SourceObject: %s with %d DesiredTrapEffects"), 
                       *UsedItem->ItemName.ToString(), UsedItem->DesiredTrapEffects.Num());
                
                if (UsedItem->DesiredTrapEffects.Num() > 0)
                {
                    Trap->ItemEffects = UsedItem->DesiredTrapEffects;
                    UE_LOG(LogTemp, Warning, TEXT("🎯 Trap configured with %d CUSTOM effects from SourceObject"), 
                           UsedItem->DesiredTrapEffects.Num());
                    
                    // 각 효과 클래스 이름 로그
                    for (int32 i = 0; i < UsedItem->DesiredTrapEffects.Num(); i++)
                    {
                        if (UsedItem->DesiredTrapEffects[i])
                        {
                            UE_LOG(LogTemp, Warning, TEXT("  🔥 Effect[%d]: %s"), i, *UsedItem->DesiredTrapEffects[i]->GetName());
                        }
                    }
                    return;
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ SourceObject is not ACYItemBase. Object class: %s"), 
                       *SourceObjectPtr->GetClass()->GetName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ SourceObject.Get() returned NULL"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ CurrentSpec or SourceObject is invalid"));
    }

    // 기본 효과 사용
    UE_LOG(LogTemp, Warning, TEXT("❌ No custom effects found, using default ImmobilizeTrap effect"));
}

void UGA_PlaceTrap::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    FGameplayEffectSpecHandle CooldownSpec = MakeOutgoingGameplayEffectSpec(UGE_TrapPlaceCooldown::StaticClass(), 1);
    if (CooldownSpec.IsValid())
    {
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpec);
    }
}