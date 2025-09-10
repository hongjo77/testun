#pragma once

#include "CoreMinimal.h"
#include "Items/CYItemBase.h"
#include "CYWeaponBase.generated.h"

UCLASS()
class CATCHME_API ACYWeaponBase : public ACYItemBase
{
	GENERATED_BODY()

public:
	ACYWeaponBase();

	// Weapon Stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float AttackRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float AttackCooldown = 1.0f;

	// Override pickup behavior
	virtual void OnPickup(ACYPlayerCharacter* Character) override;

	// Equip/Unequip
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Equip(ACYPlayerCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Unequip();

protected:
	UPROPERTY()
	ACYPlayerCharacter* OwningCharacter;
};