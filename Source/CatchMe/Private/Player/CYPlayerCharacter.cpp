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

        // Apply default effects
        for (TSubclassOf<UGameplayEffect> EffectClass : DefaultEffects)
        {
            if (EffectClass)
            {
                FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
                ContextHandle.AddSourceObject(this);
                
                FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
                    EffectClass, 1, ContextHandle
                );
                
                AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
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
}

void ACYPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckForNearbyItems();
}

void ACYPlayerCharacter::CheckForNearbyItems()
{
    // 주변 아이템 체크 로직
    // Line trace or overlap으로 구현
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