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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNearbyItemChanged, class ACYItemBase*, NearbyItem);

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
    UFUNCTION(BlueprintCallable, Category = "Input")
    void Move(const FVector2D& Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void Look(const FVector2D& Value);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void InteractPressed();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void AttackPressed();

    // Item System
    UFUNCTION(Server, Reliable, Category = "Item")
    void ServerPickupItem(ACYItemBase* Item);

    UFUNCTION(BlueprintCallable, Category = "Item")
    void EquipWeapon(class ACYWeaponBase* Weapon);

    // Inventory System
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    int32 InventorySize;

    UPROPERTY(ReplicatedUsing = OnRep_Inventory, BlueprintReadOnly, Category = "Inventory")
    TArray<ACYItemBase*> Inventory;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 FindEmptyInventorySlot() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseInventoryItem(int32 SlotIndex);

    UFUNCTION(Server, Reliable, Category = "Inventory")
    void ServerUseInventoryItem(int32 SlotIndex);

    // Utility
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PerformLineTrace(FHitResult& OutHit, float Range = 1000.0f);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNearbyItemChanged OnNearbyItemChanged;

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
    UPROPERTY(ReplicatedUsing = OnRep_NearbyItem, BlueprintReadOnly, Category = "Interaction")
    ACYItemBase* NearbyItem;

    UFUNCTION()
    void OnRep_NearbyItem();

    UFUNCTION()
    void OnRep_Inventory() { /* UI 업데이트 */ }

    void CheckForNearbyItems();

public:
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};