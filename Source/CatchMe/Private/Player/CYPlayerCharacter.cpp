#include "Player/CYPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "GAS/CYAttributeSet.h"
#include "Items/CYItemBase.h"
#include "Items/CYWeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"  // IsValid() 함수용
#include "CollisionQueryParams.h"  // 충돌 쿼리용
#include "Kismet/KismetSystemLibrary.h"
#include "GAS/CYGameplayEffects.h"

ACYPlayerCharacter::ACYPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Movement Settings
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;

    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // Spring Arm
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->TargetArmLength = 400.0f;
    SpringArmComponent->bUsePawnControlRotation = true;

    // Camera
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent);

    // GAS Components
    AbilitySystemComponent = CreateDefaultSubobject<UCYAbilitySystemComponent>(TEXT("AbilitySystem"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AttributeSet = CreateDefaultSubobject<UCYAttributeSet>(TEXT("AttributeSet"));

    // Inventory
    InventorySize = 10;
    Inventory.SetNum(InventorySize);
}

void ACYPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ACYPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Server GAS init
    if (AbilitySystemComponent)
    {
        InitializeAbilitySystem();
    }
}

void ACYPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // Client GAS init
    if (AbilitySystemComponent)
    {
        InitializeAbilitySystem();
    }
}

void ACYPlayerCharacter::InitializeAbilitySystem()
{
    if (!AbilitySystemComponent) return;

    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    // Grant default abilities
    if (HasAuthority())
    {
        for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
        {
            if (AbilityClass)
            {
                AbilitySystemComponent->GiveAbility(
                    FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this)
                );
            }
        }

        FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddSourceObject(this);
        
        FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
            UGE_InitialStats::StaticClass(), 1, ContextHandle
        );
        
        if (SpecHandle.IsValid())
        {
            AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(LogTemp, Warning, TEXT("Initial stats applied to %s"), *GetName());
        }
    }
}

UAbilitySystemComponent* ACYPlayerCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ACYPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACYPlayerCharacter, CurrentWeapon);
    DOREPLIFETIME(ACYPlayerCharacter, Inventory);
    DOREPLIFETIME(ACYPlayerCharacter, NearbyItem);
}

void ACYPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckForNearbyItems();
}

void ACYPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Enhanced Input은 PlayerController에서 처리
}

// Input Functions
void ACYPlayerCharacter::Move(const FVector2D& Value)
{
    if (Controller && (Value.X != 0.0f || Value.Y != 0.0f))
    {
        // 앞/뒤 이동
        const FVector ForwardDirection = GetActorForwardVector();
        AddMovementInput(ForwardDirection, Value.Y);

        // 좌/우 이동
        const FVector RightDirection = GetActorRightVector();
        AddMovementInput(RightDirection, Value.X);
    }
}

void ACYPlayerCharacter::Look(const FVector2D& Value)
{
    if (Controller && (Value.X != 0.0f || Value.Y != 0.0f))
    {
        AddControllerYawInput(Value.X);
        AddControllerPitchInput(Value.Y);
    }
}

void ACYPlayerCharacter::InteractPressed()
{
    if (NearbyItem)
    {
        ServerPickupItem(NearbyItem);
    }
}

void ACYPlayerCharacter::AttackPressed()
{
    if (CurrentWeapon && AbilitySystemComponent)
    {
        // 무기 공격 어빌리티 실행
        AbilitySystemComponent->TryActivateAbilityByTag(
            FGameplayTag::RequestGameplayTag("Ability.Weapon.Attack")
        );
    }
}

// Item System
void ACYPlayerCharacter::CheckForNearbyItems()
{
    if (!HasAuthority()) return;

    FVector StartLocation = GetActorLocation();
    
    // UKismetSystemLibrary 사용으로 오류 방지
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);
    
    TArray<AActor*> OutActors;
    bool bHit = UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        StartLocation,
        200.0f,  // 반지름
        TArray<TEnumAsByte<EObjectTypeQuery>>(),  // 모든 오브젝트 타입
        ACYItemBase::StaticClass(),  // 특정 클래스만
        IgnoreActors,
        OutActors
    );

    ACYItemBase* ClosestItem = nullptr;
    float ClosestDistance = FLT_MAX;

    if (bHit)
    {
        for (AActor* Actor : OutActors)
        {
            if (ACYItemBase* Item = Cast<ACYItemBase>(Actor))
            {
                // 이미 픽업된 아이템은 무시
                if (Item->bIsPickedUp) continue;
                
                float Distance = FVector::Dist(StartLocation, Item->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestItem = Item;
                }
            }
        }
    }

    if (NearbyItem != ClosestItem)
    {
        NearbyItem = ClosestItem;
        OnRep_NearbyItem();
    }
}


void ACYPlayerCharacter::ServerPickupItem_Implementation(ACYItemBase* Item)
{
    if (!Item || !HasAuthority()) return;

    // 인벤토리에 빈 슬롯 찾기
    int32 EmptySlot = FindEmptyInventorySlot();
    if (EmptySlot == -1) return; // 인벤토리 가득참

    // 무기인 경우 장착
    if (ACYWeaponBase* Weapon = Cast<ACYWeaponBase>(Item))
    {
        EquipWeapon(Weapon);
    }
    else
    {
        // 일반 아이템인 경우 인벤토리에 추가
        Inventory[EmptySlot] = Item;
        Item->OnPickup(this);
    }
}

void ACYPlayerCharacter::EquipWeapon(ACYWeaponBase* Weapon)
{
    if (!Weapon || !HasAuthority()) return;

    // 기존 무기 해제
    if (CurrentWeapon)
    {
        CurrentWeapon->Unequip();
        // 기존 무기를 인벤토리에 추가하거나 드랍
        int32 EmptySlot = FindEmptyInventorySlot();
        if (EmptySlot != -1)
        {
            Inventory[EmptySlot] = CurrentWeapon;
        }
    }

    CurrentWeapon = Weapon;
    Weapon->OnPickup(this);
    Weapon->Equip(this);
}

int32 ACYPlayerCharacter::FindEmptyInventorySlot() const
{
    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        if (!Inventory[i])
        {
            return i;
        }
    }
    return -1;
}

void ACYPlayerCharacter::UseInventoryItem(int32 SlotIndex)
{
    if (!HasAuthority()) return;

    if (SlotIndex < 0 || SlotIndex >= Inventory.Num()) return;
    if (!Inventory[SlotIndex]) return;

    ACYItemBase* Item = Inventory[SlotIndex];
    
    // 아이템 사용 로직 (트랩 등)
    if (Item->ItemAbility && AbilitySystemComponent)
    {
        // 어빌리티가 있는 아이템의 경우 어빌리티 실행
        FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(Item->ItemAbility);
        if (Spec)
        {
            AbilitySystemComponent->TryActivateAbility(Spec->Handle);
            
            // 일회용 아이템인 경우 제거
            if (Item->ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag("Item.Consumable")))
            {
                Inventory[SlotIndex] = nullptr;
            }
        }
    }
}

void ACYPlayerCharacter::ServerUseInventoryItem_Implementation(int32 SlotIndex)
{
    UseInventoryItem(SlotIndex);
}

bool ACYPlayerCharacter::PerformLineTrace(FHitResult& OutHit, float Range)
{
    FVector Start = CameraComponent->GetComponentLocation();
    FVector End = Start + (CameraComponent->GetForwardVector() * Range);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    return GetWorld()->LineTraceSingleByChannel(
        OutHit, Start, End, ECC_Visibility, Params
    );
}

// Rep Notifies
void ACYPlayerCharacter::OnRep_CurrentWeapon()
{
    // 클라이언트에서 무기 시각 업데이트
    if (CurrentWeapon)
    {
        CurrentWeapon->Equip(this);
    }
}

void ACYPlayerCharacter::OnRep_NearbyItem()
{
    // UI 업데이트 (블루프린트에서 바인딩)
    OnNearbyItemChanged.Broadcast(NearbyItem);
}