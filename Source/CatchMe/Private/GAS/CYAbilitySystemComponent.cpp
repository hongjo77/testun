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