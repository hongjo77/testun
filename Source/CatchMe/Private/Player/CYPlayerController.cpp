#include "Player/CYPlayerController.h"
#include "Player/CYPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/CYInventoryComponent.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYItemBase.h"
#include "Items/CYWeaponBase.h"
#include "CYInventoryTypes.h" // ✅ 새로운 타입 시스템

ACYPlayerController::ACYPlayerController()
{
    bReplicates = true;
}

void ACYPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer) return; // 데디케이티드 서버에서는 LocalPlayer 없음

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
        
        // 인벤토리 액션들
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
        // 레거시 Input 백업
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
    // 서버에 인벤토리 표시 요청
    ServerDisplayInventory();

    // 클라이언트에서도 인벤토리 표시
    if (IsLocalController())
    {
        DisplayInventoryOnClient();
    }

    // 무기 공격
    ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn());
    if (PlayerCharacter && PlayerCharacter->WeaponComponent && PlayerCharacter->WeaponComponent->CurrentWeapon)
    {
        ServerAttackPressed();
    }
}

void ACYPlayerController::ServerAttackPressed_Implementation()
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->AttackPressed();
    }
}

// ============ 개선된 인벤토리 슬롯 시스템 ============

void ACYPlayerController::UseInventorySlot1() { UseInventorySlotByKey(1); }
void ACYPlayerController::UseInventorySlot2() { UseInventorySlotByKey(2); }
void ACYPlayerController::UseInventorySlot3() { UseInventorySlotByKey(3); }
void ACYPlayerController::UseInventorySlot4() { UseInventorySlotByKey(4); }
void ACYPlayerController::UseInventorySlot5() { UseInventorySlotByKey(5); }
void ACYPlayerController::UseInventorySlot6() { UseInventorySlotByKey(6); }
void ACYPlayerController::UseInventorySlot7() { UseInventorySlotByKey(7); }
void ACYPlayerController::UseInventorySlot8() { UseInventorySlotByKey(8); }
void ACYPlayerController::UseInventorySlot9() { UseInventorySlotByKey(9); }

void ACYPlayerController::UseInventorySlotByKey(int32 KeyNumber)
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        if (!PlayerCharacter->InventoryComponent) return;

        // ✅ 개선된 시스템: 하드코딩 대신 타입 안전한 변환
        int32 SlotIndex = UInventorySlotUtils::KeyToSlotIndex(KeyNumber);
        if (SlotIndex >= 0)
        {
            PlayerCharacter->UseInventorySlot(SlotIndex);
        }
    }
}

void ACYPlayerController::UseInventorySlot(int32 SlotIndex)
{
    if (ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn()))
    {
        PlayerCharacter->UseInventorySlot(SlotIndex);
    }
}

// ============ 인벤토리 화면 표시 (기존 기능 유지) ============

void ACYPlayerController::DisplayInventoryOnClient()
{
    ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn());
    if (!PlayerCharacter || !PlayerCharacter->InventoryComponent || !GEngine) return;

    UCYInventoryComponent* InventoryComp = PlayerCharacter->InventoryComponent;
    UCYWeaponComponent* WeaponComp = PlayerCharacter->WeaponComponent;

    // 기존 메시지 제거
    GEngine->ClearOnScreenDebugMessages();

    // 인벤토리 상태 표시
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("=== 📦 INVENTORY STATUS ==="));
    
    // 무기 슬롯 (1~3번 키)
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("🗡️ WEAPONS (Keys 1-3):"));
    for (int32 i = 0; i < InventoryComp->WeaponSlots.Num(); ++i)
    {
        FString WeaponInfo;
        if (InventoryComp->WeaponSlots[i])
        {
            WeaponInfo = FString::Printf(TEXT("  [%d] %s x%d"), 
                i + 1, 
                *InventoryComp->WeaponSlots[i]->ItemName.ToString(), 
                InventoryComp->WeaponSlots[i]->ItemCount
            );
            
            if (WeaponComp && WeaponComp->CurrentWeapon == InventoryComp->WeaponSlots[i])
            {
                WeaponInfo += TEXT(" ⭐ EQUIPPED");
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, WeaponInfo);
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, WeaponInfo);
            }
        }
        else
        {
            WeaponInfo = FString::Printf(TEXT("  [%d] Empty"), i + 1);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, WeaponInfo);
        }
    }
    
    // 아이템 슬롯 (4~9번 키)
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("🎒 ITEMS (Keys 4-9):"));
    int32 MaxDisplayItems = FMath::Min(6, InventoryComp->ItemSlots.Num());
    for (int32 i = 0; i < MaxDisplayItems; ++i)
    {
        FString ItemInfo;
        if (InventoryComp->ItemSlots[i])
        {
            ItemInfo = FString::Printf(TEXT("  [%d] %s x%d"), 
                i + 4, 
                *InventoryComp->ItemSlots[i]->ItemName.ToString(), 
                InventoryComp->ItemSlots[i]->ItemCount
            );
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, ItemInfo);
        }
        else
        {
            ItemInfo = FString::Printf(TEXT("  [%d] Empty"), i + 4);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Silver, ItemInfo);
        }
    }
    
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("=================="));
}

void ACYPlayerController::ServerDisplayInventory_Implementation()
{
    ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn());
    if (!PlayerCharacter || !PlayerCharacter->InventoryComponent) return;

    UCYInventoryComponent* InventoryComp = PlayerCharacter->InventoryComponent;
    UCYWeaponComponent* WeaponComp = PlayerCharacter->WeaponComponent;

    UE_LOG(LogTemp, Warning, TEXT("=== 📦 SERVER INVENTORY STATUS ==="));
    
    // 무기 슬롯
    UE_LOG(LogTemp, Warning, TEXT("🗡️ WEAPONS (Keys 1-3):"));
    for (int32 i = 0; i < InventoryComp->WeaponSlots.Num(); ++i)
    {
        if (InventoryComp->WeaponSlots[i])
        {
            FString WeaponInfo = FString::Printf(TEXT("  [%d] %s x%d"), 
                i + 1, 
                *InventoryComp->WeaponSlots[i]->ItemName.ToString(), 
                InventoryComp->WeaponSlots[i]->ItemCount
            );
            
            ACYWeaponBase* SlotWeapon = Cast<ACYWeaponBase>(InventoryComp->WeaponSlots[i]);
            if (WeaponComp && WeaponComp->CurrentWeapon == SlotWeapon)
            {
                WeaponInfo += TEXT(" ⭐ EQUIPPED");
            }
            UE_LOG(LogTemp, Warning, TEXT("%s"), *WeaponInfo);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  [%d] Empty"), i + 1);
        }
    }
    
    // 아이템 슬롯
    UE_LOG(LogTemp, Warning, TEXT("🎒 ITEMS (Keys 4-9):"));
    int32 MaxDisplayItems = FMath::Min(6, InventoryComp->ItemSlots.Num());
    for (int32 i = 0; i < MaxDisplayItems; ++i)
    {
        if (InventoryComp->ItemSlots[i])
        {
            FString ItemInfo = FString::Printf(TEXT("  [%d] %s x%d"), 
                i + 4, 
                *InventoryComp->ItemSlots[i]->ItemName.ToString(), 
                InventoryComp->ItemSlots[i]->ItemCount
            );
            UE_LOG(LogTemp, Warning, TEXT("%s"), *ItemInfo);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("  [%d] Empty"), i + 4);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=================="));
}