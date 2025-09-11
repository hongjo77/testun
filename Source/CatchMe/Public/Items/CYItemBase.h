#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "CYItemBase.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class ACYPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerNearItem, ACYItemBase*, Item, bool, bNear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPickedUp, ACYItemBase*, Item, ACYPlayerCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDropped, ACYItemBase*, Item, ACYPlayerCharacter*, Character);

UCLASS(Abstract)
class CATCHME_API ACYItemBase : public AActor
{
    GENERATED_BODY()

public:
    ACYItemBase();

    // Item Info
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemDescription;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemTag;

    // GAS Integration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TSubclassOf<UGameplayAbility> ItemAbility;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TArray<TSubclassOf<UGameplayEffect>> ItemEffects;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ItemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    // State
    UPROPERTY(ReplicatedUsing = OnRep_bIsPickedUp, BlueprintReadOnly, Category = "State")
    bool bIsPickedUp;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerNearItem OnPlayerNearItem;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnItemPickedUp OnItemPickedUp;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnItemDropped OnItemDropped;

    // Pickup/Drop
    UFUNCTION(BlueprintCallable, Category = "Item")
    virtual void OnPickup(ACYPlayerCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Item")
    virtual void OnDrop(ACYPlayerCharacter* Character);

    UFUNCTION(Server, Reliable)
    void ServerPickup(ACYPlayerCharacter* Character);

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnRep_bIsPickedUp();

    // 아이템 어빌리티 핸들 (제거할 때 사용)
    FGameplayAbilitySpecHandle ItemAbilityHandle;
};