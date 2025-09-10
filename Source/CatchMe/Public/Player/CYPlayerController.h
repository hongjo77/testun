#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CYPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class CATCHME_API ACYPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACYPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Enhanced Input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	// Input Callbacks
	void Move(const struct FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void JumpPressed();
	void JumpReleased();
	void InteractPressed();
	void AttackPressed();
};