// CYPlayerCharacter.h - 중복 방지 플래그 추가
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "CYPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCYAbilitySystemComponent;
class UCYAttributeSet;
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

    // Camera Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent;

    // GAS Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    UCYAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    UCYAttributeSet* AttributeSet;

    // Gameplay Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCYInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCYItemInteractionComponent* ItemInteractionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCYWeaponComponent* WeaponComponent;

    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // Input Functions
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
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // GAS 초기화
    void InitializeAbilitySystem();
    void GrantDefaultAbilities();
    void ApplyInitialStats();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

    // ✅ 어빌리티 중복 등록 방지 플래그
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "GAS")
    bool bAbilitiesGranted = false;
};