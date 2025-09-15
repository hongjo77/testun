#include "Player/CYPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "GAS/CYAttributeSet.h"
#include "GAS/CYGameplayEffects.h"
#include "Components/CYInventoryComponent.h"
#include "Components/CYItemInteractionComponent.h"
#include "Components/CYWeaponComponent.h"
#include "CYGameplayTags.h"
#include "Abilities/GA_WeaponAttack.h"
#include "Abilities/GA_PlaceTrap.h"
#include "Net/UnrealNetwork.h"

ACYPlayerCharacter::ACYPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // 기본 설정
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 캐릭터 이동 설정
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // 카메라 시스템
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->TargetArmLength = 400.0f;
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = false;

    // GAS 시스템
    AbilitySystemComponent = CreateDefaultSubobject<UCYAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AttributeSet = CreateDefaultSubobject<UCYAttributeSet>(TEXT("AttributeSet"));

    // 게임플레이 컴포넌트들
    InventoryComponent = CreateDefaultSubobject<UCYInventoryComponent>(TEXT("InventoryComponent"));
    ItemInteractionComponent = CreateDefaultSubobject<UCYItemInteractionComponent>(TEXT("ItemInteractionComponent"));
    WeaponComponent = CreateDefaultSubobject<UCYWeaponComponent>(TEXT("WeaponComponent"));

    // 기본 어빌리티
    DefaultAbilities.Add(UGA_WeaponAttack::StaticClass());
    DefaultAbilities.Add(UGA_PlaceTrap::StaticClass());
}

void ACYPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        InitializeAbilitySystem();
    }
}

void ACYPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACYPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    if (HasAuthority())
    {
        InitializeAbilitySystem();
    }
}

void ACYPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitializeAbilitySystem();
}

void ACYPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

UAbilitySystemComponent* ACYPlayerCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

// ============ 입력 처리 함수들 ============

void ACYPlayerCharacter::Move(const FVector2D& Value)
{
    if (Controller != nullptr && !Value.IsZero())
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, Value.Y);
        AddMovementInput(RightDirection, Value.X);
    }
}

void ACYPlayerCharacter::Look(const FVector2D& Value)
{
    if (Controller != nullptr && !Value.IsZero())
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
    if (WeaponComponent)
    {
        WeaponComponent->PerformAttack();
    }
}

void ACYPlayerCharacter::UseInventorySlot(int32 SlotIndex)
{
    if (InventoryComponent)
    {
        InventoryComponent->UseItem(SlotIndex);
    }
}

// ============ GAS 초기화 ============

void ACYPlayerCharacter::InitializeAbilitySystem()
{
    if (!AbilitySystemComponent) return;

    // 수정된 부분: 타입 에러 완전 해결
    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    if (HasAuthority())
    {
        GrantDefaultAbilities();
        ApplyInitialStats();
    }
}

void ACYPlayerCharacter::GrantDefaultAbilities()
{
    if (!AbilitySystemComponent || !HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("🎯 GrantDefaultAbilities: %d abilities to grant"), DefaultAbilities.Num());

    for (int32 i = 0; i < DefaultAbilities.Num(); ++i)
    {
        TSubclassOf<UGameplayAbility>& AbilityClass = DefaultAbilities[i];
        if (AbilityClass)
        {
            FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
            FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
            
            UE_LOG(LogTemp, Warning, TEXT("✅ Granted ability: %s (Handle valid: %s)"), 
                   *AbilityClass->GetName(),
                   Handle.IsValid() ? TEXT("true") : TEXT("false"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ DefaultAbilities[%d] is null!"), i);
        }
    }
}

void ACYPlayerCharacter::ApplyInitialStats()
{
    if (!AbilitySystemComponent || !HasAuthority()) return;

    FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
    EffectContext.AddSourceObject(this);
    
    FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
        UGE_InitialStats::StaticClass(), 1, EffectContext);
    
    if (SpecHandle.IsValid())
    {
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}