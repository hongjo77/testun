// CYPlayerController.h - 입력 중복 방지 플래그 추가
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

    // Enhanced Input 설정
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* PrimaryAttackAction;

    // 인벤토리 입력 액션들
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem1Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem2Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem3Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem4Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem5Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem6Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem7Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem8Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseItem9Action;

    // ✅ 입력 중복 방지 플래그들
    UPROPERTY(BlueprintReadOnly, Category = "Input")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    bool bIsUsingItem = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float InputCooldownTime = 0.1f;

    // ✅ 타이머 핸들들
    FTimerHandle AttackCooldownTimer;
    FTimerHandle ItemUseCooldownTimer;

    // 입력 콜백 함수들
    void Move(const struct FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void JumpPressed();
    void JumpReleased();
    void InteractPressed();
    void PrimaryAttackPressed();

    // 서버 RPC 함수들
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

    // 핵심 함수
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseInventorySlotByKey(int32 KeyNumber);

    // 인벤토리 화면 표시
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DisplayInventoryOnClient();
};