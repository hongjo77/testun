#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CYAbilitySystemComponent.generated.h"

UCLASS()
class CATCHME_API UCYAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UCYAbilitySystemComponent();

	// 아이템 어빌리티 관리
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	FGameplayAbilitySpecHandle GiveItemAbility(TSubclassOf<class UGameplayAbility> AbilityClass, int32 Level = 1);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void RemoveItemAbility(FGameplayAbilitySpecHandle& Handle);

	// 태그로 어빌리티 실행
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool TryActivateAbilityByTag(FGameplayTag AbilityTag);
};