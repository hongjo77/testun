// Item/CYDamageTrap.h
#pragma once

#include "CoreMinimal.h"
#include "Item/CYTrapBase.h"
#include "CYDamageTrap.generated.h"

UCLASS()
class CATCHME_API ACYDamageTrap : public ACYTrapBase
{
	GENERATED_BODY()

public:
	ACYDamageTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Settings")
	float TrapDamage = 40.0f;

protected:
	virtual void ApplyTrapEffect(AActor* Target) override;
};