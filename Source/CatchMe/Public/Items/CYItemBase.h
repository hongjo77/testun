#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "CYItemBase.generated.h"

class UGameplayAbility;
class UGameplayEffect;

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

    // Pickup
    UFUNCTION(BlueprintCallable, Category = "Item")
    virtual void OnPickup(class ACYPlayerCharacter* Character);

    UFUNCTION(Server, Reliable)
    void ServerPickup(ACYPlayerCharacter* Character);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};