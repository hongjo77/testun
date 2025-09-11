#include "Player/CYPlayerController.h"
#include "Player/CYPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ACYPlayerController::ACYPlayerController()
{
    bReplicates = true;
}

void ACYPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ACYPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // Movement
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACYPlayerController::Move);
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACYPlayerController::Look);
        
        // Actions
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACYPlayerController::JumpPressed);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACYPlayerController::JumpReleased);
        EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ACYPlayerController::InteractPressed);
        EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &ACYPlayerController::AttackPressed);

        // 인벤토리 슬롯들 (개별 바인딩)
        if (UseItem1Action)
            EnhancedInput->BindAction(UseItem1Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot1);
        if (UseItem2Action)
            EnhancedInput->BindAction(UseItem2Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot2);
        if (UseItem3Action)
            EnhancedInput->BindAction(UseItem3Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot3);
        if (UseItem4Action)
            EnhancedInput->BindAction(UseItem4Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot4);
        if (UseItem5Action)
            EnhancedInput->BindAction(UseItem5Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot5);
        if (UseItem6Action)
            EnhancedInput->BindAction(UseItem6Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot6);
        if (UseItem7Action)
            EnhancedInput->BindAction(UseItem7Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot7);
        if (UseItem8Action)
            EnhancedInput->BindAction(UseItem8Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot8);
        if (UseItem9Action)
            EnhancedInput->BindAction(UseItem9Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot9);
    }
}

void ACYPlayerController::Move(const FInputActionValue& Value)
{
    // ❌ Character → ✅ PlayerCharacter (변수명 변경)
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->Move(Value.Get<FVector2D>());
    }
}

void ACYPlayerController::Look(const FInputActionValue& Value)
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->Look(Value.Get<FVector2D>());
    }
}

void ACYPlayerController::JumpPressed()
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->Jump();
    }
}

void ACYPlayerController::JumpReleased()
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->StopJumping();
    }
}

void ACYPlayerController::InteractPressed()
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->InteractPressed();
    }
}

void ACYPlayerController::AttackPressed()
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->AttackPressed();
    }
}

// 인벤토리 슬롯 함수 구현들
void ACYPlayerController::UseInventorySlot1() { UseInventorySlot(0); }
void ACYPlayerController::UseInventorySlot2() { UseInventorySlot(1); }
void ACYPlayerController::UseInventorySlot3() { UseInventorySlot(2); }
void ACYPlayerController::UseInventorySlot4() { UseInventorySlot(3); }
void ACYPlayerController::UseInventorySlot5() { UseInventorySlot(4); }
void ACYPlayerController::UseInventorySlot6() { UseInventorySlot(5); }
void ACYPlayerController::UseInventorySlot7() { UseInventorySlot(6); }
void ACYPlayerController::UseInventorySlot8() { UseInventorySlot(7); }
void ACYPlayerController::UseInventorySlot9() { UseInventorySlot(8); }

void ACYPlayerController::UseInventorySlot(int32 SlotIndex)
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->ServerUseInventoryItem(SlotIndex);
    }
}