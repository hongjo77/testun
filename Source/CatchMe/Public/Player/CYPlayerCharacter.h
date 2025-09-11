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
class UCYInventoryComponent;
class UCYItemInteractionComponent;
class UCYWeaponComponent;

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

    // ✅ 새로운 컴포넌트들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCYInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCYItemInteractionComponent* ItemInteractionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCYWeaponComponent* WeaponComponent;

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // Input Functions (컴포넌트에 위임)
    UFUNCTION(BlueprintCallable, Category = "Input")
    void Move(const FVector2D& Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void Look(const FVector2D& Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void InteractPressed();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void AttackPressed();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void UseInventorySlot(int32 SlotIndex);

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

    void InitializeAbilitySystem();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};