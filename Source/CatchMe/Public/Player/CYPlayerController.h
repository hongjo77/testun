#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "CYPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ACYPlayerCharacter;

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
    UInputAction* PrimaryAttackAction;

    // 인벤토리 액션들
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem1Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem2Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem3Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem4Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem5Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem6Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem7Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem8Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem9Action;

    // Input Callbacks
    void Move(const struct FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void JumpPressed();
    void JumpReleased();
    void InteractPressed();
    void PrimaryAttackPressed(); // 클라이언트에서 호출

    // 🔥 RPC 함수들
    UFUNCTION(Server, Reliable)
    void ServerAttackPressed();
    UFUNCTION(Server, Reliable)
    void ServerDisplayInventory();

    // 인벤토리 슬롯 함수들
    void UseInventorySlot1();
    void UseInventorySlot2();
    void UseInventorySlot3();
    void UseInventorySlot4();
    void UseInventorySlot5();
    void UseInventorySlot6();
    void UseInventorySlot7();
    void UseInventorySlot8();
    void UseInventorySlot9();

    void DisplayInventoryOnClient();

private:
    void UseInventorySlot(int32 SlotIndex);
};