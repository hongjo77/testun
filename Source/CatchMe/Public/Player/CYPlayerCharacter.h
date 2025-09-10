#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "CYPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAbilitySystemComponent;
class UCYAttributeSet;
class UCYAbilitySystemComponent;
class UGameplayEffect;
class UGameplayAbility;

UCLASS()
class CATCHME_API ACYPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ACYPlayerCharacter();

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    UCYAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    UCYAttributeSet* AttributeSet;

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // Input
    void Move(const FVector2D& Value);
    void Look(const FVector2D& Value);
    void InteractPressed();
    void AttackPressed();

    // Item System
    UFUNCTION(BlueprintCallable, Category = "Item")
    void PickupItem(class ACYItemBase* Item);

    UFUNCTION(BlueprintCallable, Category = "Item")
    void EquipWeapon(class ACYWeaponBase* Weapon);

    // Utility
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PerformLineTrace(FHitResult& OutHit, float Range = 1000.0f);

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

    // GAS Initialization
    void InitializeAbilitySystem();

    // Default Abilities
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

    // Default Effects (초기 스탯)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

    // Current Equipment
    UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Equipment")
    ACYWeaponBase* CurrentWeapon;

    UFUNCTION()
    void OnRep_CurrentWeapon();

    // Interaction
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    ACYItemBase* NearbyItem;

    void CheckForNearbyItems();

public:
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};