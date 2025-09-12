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
    UE_LOG(LogTemp, Warning, TEXT("=== CYPlayerController::BeginPlay START ==="));
    UE_LOG(LogTemp, Warning, TEXT("IsLocalController: %s"), IsLocalController() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("HasAuthority: %s"), HasAuthority() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("GetNetMode: %d"), (int32)GetNetMode());
    
    Super::BeginPlay();

    // 🔍 Enhanced Input 실패 원인 진단
    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    UE_LOG(LogTemp, Warning, TEXT("LocalPlayer: %s"), LocalPlayer ? TEXT("EXISTS") : TEXT("NULL"));
    
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("🚨 NO LocalPlayer - This is why Enhanced Input fails!"));
        UE_LOG(LogTemp, Error, TEXT("🚨 Dedicated Server has no LocalPlayer"));
        UE_LOG(LogTemp, Warning, TEXT("✅ Using Legacy Input instead"));
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Found EnhancedInputLocalPlayerSubsystem"));
        
        if (DefaultMappingContext)
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Adding DefaultMappingContext"));
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ DefaultMappingContext is NULL!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to get EnhancedInputLocalPlayerSubsystem"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== CYPlayerController::BeginPlay END ==="));
}

void ACYPlayerController::SetupInputComponent()
{
    UE_LOG(LogTemp, Error, TEXT("🔥🔥🔥 SETUP INPUT COMPONENT CALLED 🔥🔥🔥"));
    
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("Enhanced Input Component found"));
        
        UE_LOG(LogTemp, Warning, TEXT("PrimaryAttackAction: %s"), PrimaryAttackAction ? TEXT("VALID") : TEXT("NULL"));
        
        if (PrimaryAttackAction)
        {
            EnhancedInput->BindAction(PrimaryAttackAction, ETriggerEvent::Started, this, &ACYPlayerController::PrimaryAttackPressed);
            UE_LOG(LogTemp, Warning, TEXT("PrimaryAttackAction bound successfully"));
        }
        
        // Movement
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

    // 🔥 레거시 Input 바인딩 (Enhanced Input 실패 시 대안)
    if (InputComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Adding legacy input bindings"));
        
        // 🔥 Attack 바인딩 - 여러 키로 테스트
        InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ACYPlayerController::PrimaryAttackPressed);
        InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ACYPlayerController::PrimaryAttackPressed);
        InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACYPlayerController::PrimaryAttackPressed);
        
        // 기존 바인딩들
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
        
        UE_LOG(LogTemp, Warning, TEXT("✅ LEGACY BINDINGS COMPLETE: F, SpaceBar, LeftMouse → Attack"));
        
        // 🔥 화면 메시지로 확인
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow,
                TEXT("Input Setup Complete - F/Space/Mouse = Attack!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ NO InputComponent!"));
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

// 🔥 클라이언트에서 호출되는 함수
void ACYPlayerController::PrimaryAttackPressed()
{
    // 🔥 클라이언트 로그 (클라이언트 콘솔에만 표시)
    UE_LOG(LogTemp, Error, TEXT("🚀 CLIENT: PRIMARY ATTACK PRESSED!!!"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            TEXT("CLIENT: ATTACK KEY PRESSED!"));
    }
    
    // 🔥 서버로 RPC 전송
    ServerAttackPressed();
}

// 🔥 서버에서 실행되는 RPC 함수
void ACYPlayerController::ServerAttackPressed_Implementation()
{
    // 🔥 서버 로그 (데디케이티드 서버 콘솔에 표시)
    UE_LOG(LogTemp, Error, TEXT("🔥 SERVER: Attack RPC received from %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("NULL"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            TEXT("SERVER: Attack RPC Received!"));
    }
    
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        UE_LOG(LogTemp, Warning, TEXT("SERVER: Calling PlayerCharacter->AttackPressed"));
        PlayerCharacter->AttackPressed();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SERVER: GetPawn() is not CYPlayerCharacter"));
    }
}

// 키 매핑: 1~3번은 무기, 4~9번은 아이템
void ACYPlayerController::UseInventorySlot1() { UseInventorySlot(1000); } // 무기 슬롯 0
void ACYPlayerController::UseInventorySlot2() { UseInventorySlot(1001); } // 무기 슬롯 1
void ACYPlayerController::UseInventorySlot3() { UseInventorySlot(1002); } // 무기 슬롯 2
void ACYPlayerController::UseInventorySlot4() { UseInventorySlot(0); }    // 아이템 슬롯 0
void ACYPlayerController::UseInventorySlot5() { UseInventorySlot(1); }    // 아이템 슬롯 1
void ACYPlayerController::UseInventorySlot6() { UseInventorySlot(2); }    // 아이템 슬롯 2
void ACYPlayerController::UseInventorySlot7() { UseInventorySlot(3); }    // 아이템 슬롯 3
void ACYPlayerController::UseInventorySlot8() { UseInventorySlot(4); }    // 아이템 슬롯 4
void ACYPlayerController::UseInventorySlot9() { UseInventorySlot(5); }    // 아이템 슬롯 5

void ACYPlayerController::UseInventorySlot(int32 SlotIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("PlayerController::UseInventorySlot %d"), SlotIndex);
    
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->UseInventorySlot(SlotIndex);
    }
}