#include "GAS/CYAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"

UCYAbilitySystemComponent::UCYAbilitySystemComponent()
{
    // 리플리케이션 설정
    SetIsReplicated(true);
    SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

FGameplayAbilitySpecHandle UCYAbilitySystemComponent::GiveItemAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    if (!AbilityClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("GiveItemAbility: AbilityClass is null"));
        return FGameplayAbilitySpecHandle();
    }

    FGameplayAbilitySpec AbilitySpec(AbilityClass, Level, INDEX_NONE, GetOwnerActor());
    
    // 서버에서만 어빌리티 부여
    if (GetOwnerRole() == ROLE_Authority)
    {
        return GiveAbility(AbilitySpec);
    }

    return FGameplayAbilitySpecHandle();
}

void UCYAbilitySystemComponent::RemoveItemAbility(FGameplayAbilitySpecHandle& Handle)
{
    if (!Handle.IsValid())
    {
        return;
    }

    // 서버에서만 어빌리티 제거
    if (GetOwnerRole() == ROLE_Authority)
    {
        ClearAbility(Handle);
        // 핸들 무효화
        Handle = FGameplayAbilitySpecHandle();
    }
}

bool UCYAbilitySystemComponent::TryActivateAbilityByTag(FGameplayTag AbilityTag)
{
    if (!AbilityTag.IsValid())
    {
        return false;
    }

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(AbilityTag);

    return TryActivateAbilitiesByTag(TagContainer);
}

// ✅ SourceObject와 함께 어빌리티 활성화
bool UCYAbilitySystemComponent::TryActivateAbilityByTagWithSource(FGameplayTag AbilityTag, UObject* SourceObject)
{
    if (!AbilityTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("TryActivateAbilityByTagWithSource: Invalid AbilityTag"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("🎯 TryActivateAbilityByTagWithSource: %s with SourceObject: %s"), 
           *AbilityTag.ToString(), SourceObject ? *SourceObject->GetName() : TEXT("NULL"));

    // ✅ 태그로 어빌리티를 찾기
    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(AbilityTag);

    TArray<FGameplayAbilitySpec*> AbilitySpecs;
    GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, AbilitySpecs);

    if (AbilitySpecs.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("🎯 No abilities found for tag: %s"), *AbilityTag.ToString());
        return false;
    }

    // 첫 번째로 찾은 어빌리티 사용
    FGameplayAbilitySpec* AbilitySpec = AbilitySpecs[0];
    if (!AbilitySpec)
    {
        UE_LOG(LogTemp, Error, TEXT("🎯 AbilitySpec is null"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("🎯 Found ability: %s"), *AbilitySpec->Ability->GetName());

    if (SourceObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("🎯 Setting SourceObject: %s"), *SourceObject->GetName());
        AbilitySpec->SourceObject = SourceObject;
    }

    // ✅ 어빌리티 활성화
    bool bResult = TryActivateAbility(AbilitySpec->Handle);
    UE_LOG(LogTemp, Warning, TEXT("🎯 TryActivateAbility result: %s"), bResult ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    return bResult;
}