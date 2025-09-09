// PlayerCharacter.cpp
#include "Player/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Item/CYTrapBase.h"
#include "Systems/CYEffectManagerComponent.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 캐릭터 무브먼트 설정
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;

    // 컨트롤러 회전 설정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    // 스프링 암 컴포넌트 생성
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->TargetArmLength = 400.0f;
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->bInheritPitch = true;
    SpringArmComponent->bInheritYaw = true;
    SpringArmComponent->bInheritRoll = false;

    // 카메라 컴포넌트 생성
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);

    // 시스템 컴포넌트들 생성
    EffectManager = CreateDefaultSubobject<UCYEffectManagerComponent>(TEXT("EffectManager"));
    Inventory = CreateDefaultSubobject<UCYInventoryComponent>(TEXT("Inventory"));
    WeaponManager = CreateDefaultSubobject<UCYWeaponManagerComponent>(TEXT("WeaponManager"));
    TrapManager = CreateDefaultSubobject<UCYTrapManagerComponent>(TEXT("TrapManager"));

    // 네트워크 복제 설정
    bReplicates = true;
    SetReplicateMovement(true);

    // 기본 마우스 감도
    MouseSensitivity = 1.0f;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    NearbyItem = nullptr;
    
    // 효과 매니저 이벤트 구독
    if (EffectManager)
    {
        EffectManager->OnEffectApplied.AddDynamic(this, &APlayerCharacter::OnEffectApplied);
        EffectManager->OnEffectRemoved.AddDynamic(this, &APlayerCharacter::OnEffectRemoved);
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckForNearbyItems();
}

// ========================= PlayerController에서 호출되는 함수들 =========================

void APlayerCharacter::HandleMove(const FVector2D& Value)
{
    if (Controller && !Value.IsZero())
    {
        // 전진/후진 (W/S)
        if (Value.Y != 0.0f)
        {
            const FRotator Rotation = Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            AddMovementInput(Direction, Value.Y);
        }

        // 좌우 이동 (A/D)
        if (Value.X != 0.0f)
        {
            const FRotator Rotation = Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);
            const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
            AddMovementInput(Direction, Value.X);
        }
    }
}

void APlayerCharacter::HandleLook(const FVector2D& Value)
{
    if (Controller && !Value.IsZero())
    {
        // 마우스 감도 적용
        AddControllerYawInput(Value.X * MouseSensitivity);
        AddControllerPitchInput(Value.Y * MouseSensitivity);
    }
}

void APlayerCharacter::HandleJump()
{
    Jump();
}

void APlayerCharacter::HandleStopJump()
{
    StopJumping();
}

void APlayerCharacter::HandleInteract()
{
    UE_LOG(LogTemp, Warning, TEXT("HandleInteract called"));
    
    if (NearbyItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Trying to pickup: %s"), *NearbyItem->GetName());
        PickupItem(NearbyItem);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No nearby item to pickup"));
    }
}

void APlayerCharacter::HandleUseItem()
{
    // 좌클릭 - 아이템/무기 사용
    if (WeaponManager && WeaponManager->CurrentWeapon)
    {
        // 무기가 장착되어 있으면 무기 사용
        FHitResult HitResult;
        if (PerformLineTrace(HitResult))
        {
            WeaponManager->ServerUseWeapon(HitResult.Location, HitResult.GetActor());
        }
    }
    else if (Inventory)
    {
        // 무기가 없으면 인벤토리 첫 번째 슬롯 아이템 사용
        if (ACYItemBase* Item = Inventory->GetItemAtSlot(0))
        {
            // 트랩인지 확인
            if (ACYTrapBase* TrapItem = Cast<ACYTrapBase>(Item))
            {
                // 트랩 설치
                FVector PlaceLocation = GetActorLocation() + GetActorForwardVector() * 200.0f;
                TrapManager->ServerPlaceTrap(TrapItem->GetClass(), PlaceLocation, GetActorRotation());
                Inventory->ServerRemoveItem(0);
            }
            else
            {
                // 일반 아이템 사용
                Item->OnItemUsed(this);
                Inventory->ServerRemoveItem(0);
            }
        }
    }
}

// ========================= 기타 함수들 =========================

void APlayerCharacter::CheckForNearbyItems()
{
    // 근처 아이템 찾기
    TArray<FHitResult> HitResults;
    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        GetWorld(),
        StartLocation,
        EndLocation,
        InteractionRange,
        ObjectTypes,
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitResults,
        true
    );

    // 가장 가까운 아이템 찾기
    ACYItemBase* ClosestItem = nullptr;
    float ClosestDistance = InteractionRange;

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (ACYItemBase* Item = Cast<ACYItemBase>(Hit.GetActor()))
            {
                float Distance = FVector::Dist(GetActorLocation(), Item->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestItem = Item;
                }
            }
        }
    }

    // UI 업데이트
    if (NearbyItem != ClosestItem)
    {
        NearbyItem = ClosestItem;
        
        if (NearbyItem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Press E to pickup: %s"), *NearbyItem->ItemName.ToString());
        }
    }
}

void APlayerCharacter::PickupItem(ACYItemBase* Item)
{
    if (Item && Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("Calling ServerPickupItem"));
        Item->ServerPickupItem(this);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Item or Inventory is null"));
    }
}

bool APlayerCharacter::PerformLineTrace(FHitResult& HitResult, float Range)
{
    FVector StartLocation = CameraComponent->GetComponentLocation();
    FVector ForwardVector = CameraComponent->GetForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * Range);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return bHit;
}

void APlayerCharacter::OnEffectApplied(FGameplayTag EffectType, float Duration, float Magnitude)
{
    UE_LOG(LogTemp, Warning, TEXT("Player received effect: %s for %.1fs"), *EffectType.ToString(), Duration);
    
    if (EffectType == FGameplayTag::RequestGameplayTag("Effect.Immobilize"))
    {
        bCanMove = false;
        UE_LOG(LogTemp, Warning, TEXT("Player is now immobilized"));
    }
}

void APlayerCharacter::OnEffectRemoved(FGameplayTag EffectType)
{
    UE_LOG(LogTemp, Warning, TEXT("Player effect removed: %s"), *EffectType.ToString());
    
    if (EffectType == FGameplayTag::RequestGameplayTag("Effect.Immobilize"))
    {
        bCanMove = true;
        UE_LOG(LogTemp, Warning, TEXT("Player can move again"));
    }
}