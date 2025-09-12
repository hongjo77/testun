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
        // ✅ 트랩 생성 후 즉시 커스텀 효과 설정 (BeginPlay 전에)
        ConfigureTrapEffects(Trap);
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

void UGA_PlaceTrap::ConfigureTrapEffects(ACYTrapBase* Trap)
{
    if (!Trap) return;

    UE_LOG(LogTemp, Warning, TEXT("🔧 ConfigureTrapEffects: Starting trap effect configuration"));

    // ✅ 먼저 기본값 설정
    Trap->ItemEffects.Empty();
    Trap->ItemEffects.Add(UGE_ImmobilizeTrap::StaticClass());

    // ✅ 현재 어빌리티 Spec 확인
    const FGameplayAbilitySpec* CurrentSpec = GetCurrentAbilitySpec();
    if (!CurrentSpec)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GetCurrentAbilitySpec() returned NULL"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("✅ CurrentSpec found, checking SourceObject..."));

    // ✅ SourceObject 유효성 검사
    if (!CurrentSpec->SourceObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("❌ CurrentSpec->SourceObject is INVALID"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ SourceObject is VALID, trying to cast..."));

    // ✅ 캐스팅 시도
    UObject* SourceObjectPtr = CurrentSpec->SourceObject.Get();
    if (!SourceObjectPtr)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ SourceObject.Get() returned NULL - object was garbage collected?"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ SourceObject.Get() success: %s"), *SourceObjectPtr->GetName());

    ACYItemBase* UsedItem = Cast<ACYItemBase>(SourceObjectPtr);
    if (!UsedItem)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to cast SourceObject to ACYItemBase. Object class: %s"), 
               *SourceObjectPtr->GetClass()->GetName());
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ Successfully cast to ACYItemBase: %s with %d DesiredTrapEffects"), 
           *UsedItem->ItemName.ToString(), UsedItem->DesiredTrapEffects.Num());
    
    if (UsedItem->DesiredTrapEffects.Num() > 0)
    {
        Trap->ItemEffects = UsedItem->DesiredTrapEffects;
        UE_LOG(LogTemp, Warning, TEXT("🎯 Trap configured with %d CUSTOM effects from %s"), 
               UsedItem->DesiredTrapEffects.Num(), *UsedItem->ItemName.ToString());
        
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
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ DesiredTrapEffects is empty in %s"), *UsedItem->ItemName.ToString());
    }

    // 기본 효과 사용
    UE_LOG(LogTemp, Warning, TEXT("Using default ImmobilizeTrap effect"));
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