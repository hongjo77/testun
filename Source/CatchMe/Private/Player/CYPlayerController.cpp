#include "Player/CYPlayerController.h"
#include "Player/CYPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/CYInventoryComponent.h"
#include "Components/CYWeaponComponent.h"
#include "Items/CYItemBase.h"
#include "Items/CYWeaponBase.h"

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
    UE_LOG(LogTemp, Warning, TEXT("🖱️ PrimaryAttackPressed called"));

    // ✅ 항상 서버에 인벤토리 표시 요청 (서버 로그용)
    ServerDisplayInventory();

    // 클라이언트에서도 인벤토리 표시 (로컬 플레이어용)
    if (IsLocalController())
    {
        UE_LOG(LogTemp, Warning, TEXT("🖱️ PrimaryAttackPressed - IsLocalController: true"));
        DisplayInventoryOnClient();
    }

    // 무기가 있으면 서버에서 공격 처리
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

void ACYPlayerController::DisplayInventoryOnClient()
{
    UE_LOG(LogTemp, Warning, TEXT("🖱️ DisplayInventoryOnClient called"));
    
    ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn());
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No PlayerCharacter"));
        return;
    }

    UCYInventoryComponent* InventoryComp = PlayerCharacter->InventoryComponent;
    UCYWeaponComponent* WeaponComp = PlayerCharacter->WeaponComponent;
    
    if (!InventoryComp)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No InventoryComponent"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("❌ No InventoryComponent found"));
        }
        return;
    }

    if (!GEngine)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GEngine is NULL"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ All components found, displaying inventory"));

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
            
            // ✅ WeaponSlots가 ACYWeaponBase* 타입이면 Cast 불필요
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
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, ItemInfo);
        }
    }
    
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("=================="));
    UE_LOG(LogTemp, Warning, TEXT("✅ Inventory displayed successfully"));
}

void ACYPlayerController::ServerDisplayInventory_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("🖱️ ServerDisplayInventory called on server"));
    
    ACYPlayerCharacter* PlayerCharacter = Cast<ACYPlayerCharacter>(GetPawn());
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No PlayerCharacter on server"));
        return;
    }

    UCYInventoryComponent* InventoryComp = PlayerCharacter->InventoryComponent;
    UCYWeaponComponent* WeaponComp = PlayerCharacter->WeaponComponent;
    
    if (!InventoryComp)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No InventoryComponent on server"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ SERVER INVENTORY STATUS for %s:"), *PlayerCharacter->GetName());
    UE_LOG(LogTemp, Warning, TEXT("=== 📦 SERVER INVENTORY STATUS ==="));
    
    // 무기 슬롯 (1~3번 키)
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
            
            // ✅ Cast 사용해서 타입 문제 해결
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
    
    // 아이템 슬롯 (4~9번 키)
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