// CYPlayerCharacter.cpp - 어빌리티 중복 등록 방지
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

    // ✅ 기본 어빌리티 - 각각 한 번만 등록
    DefaultAbilities.Add(UGA_WeaponAttack::StaticClass());
    DefaultAbilities.Add(UGA_PlaceTrap::StaticClass());
    
    // ✅ 중복 방지를 위한 플래그
    bAbilitiesGranted = false;
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
    DOREPLIFETIME(ACYPlayerCharacter, bAbilitiesGranted); // ✅ 어빌리티 부여 상태 동기화
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

    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    if (HasAuthority() && !bAbilitiesGranted)
    {
        GrantDefaultAbilities();
        ApplyInitialStats();
        bAbilitiesGranted = true; // ✅ 중복 방지 플래그 설정
        UE_LOG(LogTemp, Warning, TEXT("✅ Abilities granted to: %s"), *GetName());
    }
}

void ACYPlayerCharacter::GrantDefaultAbilities()
{
    if (!AbilitySystemComponent || !HasAuthority() || bAbilitiesGranted) return;

    UE_LOG(LogTemp, Warning, TEXT("🎯 Granting %d default abilities to: %s"), DefaultAbilities.Num(), *GetName());

    for (int32 i = 0; i < DefaultAbilities.Num(); ++i)
    {
        TSubclassOf<UGameplayAbility>& AbilityClass = DefaultAbilities[i];
        if (AbilityClass)
        {
            // ✅ 중복 등록 방지 - 해당 태그의 어빌리티가 이미 있는지 체크
            FGameplayTag AbilityTag;
            
            // 어빌리티 클래스에 따라 태그 결정
            if (AbilityClass == UGA_WeaponAttack::StaticClass())
            {
                AbilityTag = FGameplayTag::RequestGameplayTag("Ability.Weapon.Attack");
            }
            else if (AbilityClass == UGA_PlaceTrap::StaticClass())
            {
                AbilityTag = FGameplayTag::RequestGameplayTag("Ability.Trap.Place");
            }
            
            // 해당 태그의 어빌리티가 이미 있는지 체크
            if (AbilityTag.IsValid())
            {
                FGameplayTagContainer TagContainer;
                TagContainer.AddTag(AbilityTag);
                
                TArray<FGameplayAbilitySpec*> ExistingAbilities;
                AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, ExistingAbilities);
                
                if (ExistingAbilities.Num() > 0)
                {
                    UE_LOG(LogTemp, Warning, TEXT("⚠️ Ability already exists for tag: %s, skipping"), *AbilityTag.ToString());
                    continue;
                }
            }
            
            FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
            FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
            
            UE_LOG(LogTemp, Warning, TEXT("✅ Granted ability: %s (Tag: %s)"), 
                   *AbilityClass->GetName(), 
                   AbilityTag.IsValid() ? *AbilityTag.ToString() : TEXT("None"));
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