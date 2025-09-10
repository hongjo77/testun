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
    }
}

void ACYPlayerController::Move(const FInputActionValue& Value)
{
    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        Character->Move(Value.Get<FVector2D>());
    }
}

void ACYPlayerController::Look(const FInputActionValue& Value)
{
    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        Character->Look(Value.Get<FVector2D>());
    }
}

void ACYPlayerController::InteractPressed()
{
    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        Character->InteractPressed();
    }
}

void ACYPlayerController::AttackPressed()
{
    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        Character->AttackPressed();
    }
}