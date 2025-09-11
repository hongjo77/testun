#include "Player/CYPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "GAS/CYAttributeSet.h"
#include "Components/CYInventoryComponent.h"
#include "Components/CYItemInteractionComponent.h"
#include "Components/CYWeaponComponent.h"
#include "Net/UnrealNetwork.h"
#include "GAS/CYGameplayEffects.h"

ACYPlayerCharacter::ACYPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

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

    // ✅ 새로운 컴포넌트들 생성
    InventoryComponent = CreateDefaultSubobject<UCYInventoryComponent>(TEXT("InventoryComponent"));
    ItemInteractionComponent = CreateDefaultSubobject<UCYItemInteractionComponent>(TEXT("ItemInteractionComponent"));
    WeaponComponent = CreateDefaultSubobject<UCYWeaponComponent>(TEXT("WeaponComponent"));
}

void ACYPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ACYPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (AbilitySystemComponent)
    {
        InitializeAbilitySystem();
    }
}

void ACYPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (AbilitySystemComponent)
    {
        InitializeAbilitySystem();
    }
}

void ACYPlayerCharacter::InitializeAbilitySystem()
{
    if (!AbilitySystemComponent) return;

    AbilitySystemComponent->InitAbilityActorInfo(this, this);

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
    // 컴포넌트들이 자체적으로 리플리케이션 처리
}

void ACYPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// ✅ 입력 함수들 - 컴포넌트에 위임
void ACYPlayerCharacter::Move(const FVector2D& Value)
{
    if (Controller && (Value.X != 0.0f || Value.Y != 0.0f))
    {
        const FVector ForwardDirection = GetActorForwardVector();
        AddMovementInput(ForwardDirection, Value.Y);

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
    if (ItemInteractionComponent)
    {
        ItemInteractionComponent->InteractWithNearbyItem();
    }
}

void ACYPlayerCharacter::AttackPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ACYPlayerCharacter::AttackPressed called ==="));
    
    if (WeaponComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: WeaponComponent found, calling PerformAttack"));
        WeaponComponent->PerformAttack();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: WeaponComponent is NULL!"));
        
        // ✅ 컴포넌트 존재 여부 확인
        if (InventoryComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: InventoryComponent exists"));
            InventoryComponent->PrintInventoryStatus();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: InventoryComponent is also NULL!"));
        }
    }
}

void ACYPlayerCharacter::UseInventorySlot(int32 SlotIndex)
{
    if (InventoryComponent)
    {
        InventoryComponent->ServerUseItem(SlotIndex);
    }
}