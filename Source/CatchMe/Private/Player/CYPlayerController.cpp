// PlayerController.cpp
#include "Player/CYPlayerController.h"
#include "Player/PlayerCharacter.h"
#include "InputActionValue.h"

ACYPlayerController::ACYPlayerController()
{
    // 기본 설정
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
}

void ACYPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input 서브시스템에 Input Mapping Context 추가
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (InputMappingContext)
        {
            Subsystem->AddMappingContext(InputMappingContext, 0);
        }
    }

    // 컨트롤하는 캐릭터 캐싱
    ControlledCharacter = Cast<APlayerCharacter>(GetPawn());
}

void ACYPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Enhanced Input Component로 캐스팅
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // 이동 입력 바인딩
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACYPlayerController::HandleMove);
        }

        // 시점 회전 입력 바인딩
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACYPlayerController::HandleLook);
        }

        // 점프 입력 바인딩
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACYPlayerController::HandleJump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACYPlayerController::HandleStopJump);
        }

        // 상호작용 입력 바인딩
        if (InteractAction)
        {
            UE_LOG(LogTemp, Warning, TEXT("InteractAction"));
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ACYPlayerController::HandleInteract);
        }

        // 아이템 사용 입력 바인딩
        if (UseItemAction)
        {
            EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started, this, &ACYPlayerController::HandleUseItem);
        }
    }
}

// ========================= 입력 처리 함수들 =========================

void ACYPlayerController::HandleMove(const FInputActionValue& Value)
{
    if (ControlledCharacter)
    {
        const FVector2D MovementVector = Value.Get<FVector2D>();
        ControlledCharacter->HandleMove(MovementVector);
    }
}

void ACYPlayerController::HandleLook(const FInputActionValue& Value)
{
    if (ControlledCharacter)
    {
        const FVector2D LookAxisVector = Value.Get<FVector2D>();
        ControlledCharacter->HandleLook(LookAxisVector);
    }
}

void ACYPlayerController::HandleJump()
{
    if (ControlledCharacter)
    {
        ControlledCharacter->HandleJump();
    }
}

void ACYPlayerController::HandleStopJump()
{
    if (ControlledCharacter)
    {
        ControlledCharacter->HandleStopJump();
    }
}

void ACYPlayerController::HandleInteract()
{
    if (ControlledCharacter)
    {
        ControlledCharacter->HandleInteract();
    }
}

void ACYPlayerController::HandleUseItem()
{
    if (ControlledCharacter)
    {
        ControlledCharacter->HandleUseItem();
    }
}