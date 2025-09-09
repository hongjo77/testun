// Item/CYImmobilizeTrap.h
#pragma once

#include "CoreMinimal.h"
#include "Item/CYTrapBase.h"
#include "CYImmobilizeTrap.generated.h"

UCLASS()
class CATCHME_API ACYImmobilizeTrap : public ACYTrapBase
{
	GENERATED_BODY()

public:
	ACYImmobilizeTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immobilize Settings")
	float ImmobilizeDuration = 5.0f;

protected:
	virtual void ApplyTrapEffect(AActor* Target) override;
};