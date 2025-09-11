#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CYGameplayTags.generated.h"

USTRUCT()
struct CATCHME_API FCYGameplayTags
{
	GENERATED_BODY()

	static const FCYGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeTags();

	// ============ 어빌리티 태그 ============
	FGameplayTag Ability_Weapon_Attack;
	FGameplayTag Ability_Trap_Place;

	// ============ 아이템 태그 ============
	FGameplayTag Item_Base;
	FGameplayTag Item_Weapon;
	FGameplayTag Item_Trap;
	FGameplayTag Item_Consumable;

	// ============ 상태 태그 ============
	FGameplayTag State_Attacking;
	FGameplayTag State_Stunned;
	FGameplayTag State_Dead;

	// ============ 쿨다운 태그 ============
	FGameplayTag Cooldown_Weapon_Attack;
	FGameplayTag Cooldown_Trap_Place;

	// ============ 데이터 태그 ============
	FGameplayTag Data_Damage;

private:
	static FCYGameplayTags GameplayTags;
};