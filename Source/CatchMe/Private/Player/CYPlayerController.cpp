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

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        return; // 데디케이티드 서버에서는 LocalPlayer 없음 (정상)
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
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
        // Enhanced Input 바인딩
        if (PrimaryAttackAction)
            EnhancedInput->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &ACYPlayerController::PrimaryAttackPressed);
        if (MoveAction)
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACYPlayerController::Move);
        if (LookAction)
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACYPlayerController::Look);
        if (JumpAction)
        {
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACYPlayerController::JumpPressed);
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACYPlayerController::JumpReleased);
        }
        if (InteractAction)
            EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ACYPlayerController::InteractPressed);
        
        // 인벤토리 키들
        if (UseItem1Action) EnhancedInput->BindAction(UseItem1Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot1);
        if (UseItem2Action) EnhancedInput->BindAction(UseItem2Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot2);
        if (UseItem3Action) EnhancedInput->BindAction(UseItem3Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot3);
        if (UseItem4Action) EnhancedInput->BindAction(UseItem4Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot4);
        if (UseItem5Action) EnhancedInput->BindAction(UseItem5Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot5);
        if (UseItem6Action) EnhancedInput->BindAction(UseItem6Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot6);
        if (UseItem7Action) EnhancedInput->BindAction(UseItem7Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot7);
        if (UseItem8Action) EnhancedInput->BindAction(UseItem8Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot8);
        if (UseItem9Action) EnhancedInput->BindAction(UseItem9Action, ETriggerEvent::Started, this, &ACYPlayerController::UseInventorySlot9);
    }
    else if (InputComponent)
    {
        // 레거시 Input 백업 (Enhanced Input 실패 시)
        InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACYPlayerController::PrimaryAttackPressed);
        InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ACYPlayerController::InteractPressed);
        InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ACYPlayerController::UseInventorySlot1);
        InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ACYPlayerController::UseInventorySlot2);
        InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ACYPlayerController::UseInventorySlot3);
        InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ACYPlayerController::UseInventorySlot4);
        InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ACYPlayerController::UseInventorySlot5);
        InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ACYPlayerController::UseInventorySlot6);
        InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &ACYPlayerController::UseInventorySlot7);
        InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &ACYPlayerController::UseInventorySlot8);
        InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ACYPlayerController::UseInventorySlot9);
    }
}

void ACYPlayerController::Move(const FInputActionValue& Value)
{
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

void ACYPlayerController::PrimaryAttackPressed()
{
    ServerAttackPressed();
}

void ACYPlayerController::ServerAttackPressed_Implementation()
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->AttackPressed();
    }
}

// 키 매핑: 1~3번은 무기, 4~9번은 아이템
void ACYPlayerController::UseInventorySlot1() { UseInventorySlot(1000); }
void ACYPlayerController::UseInventorySlot2() { UseInventorySlot(1001); }
void ACYPlayerController::UseInventorySlot3() { UseInventorySlot(1002); }
void ACYPlayerController::UseInventorySlot4() { UseInventorySlot(0); }
void ACYPlayerController::UseInventorySlot5() { UseInventorySlot(1); }
void ACYPlayerController::UseInventorySlot6() { UseInventorySlot(2); }
void ACYPlayerController::UseInventorySlot7() { UseInventorySlot(3); }
void ACYPlayerController::UseInventorySlot8() { UseInventorySlot(4); }
void ACYPlayerController::UseInventorySlot9() { UseInventorySlot(5); }

void ACYPlayerController::UseInventorySlot(int32 SlotIndex)
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->UseInventorySlot(SlotIndex);
    }
}