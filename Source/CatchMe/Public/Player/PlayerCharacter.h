// PlayerCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Systems/CYEffectManagerComponent.h"
#include "Systems/CYInventoryComponent.h"
#include "Systems/CYWeaponManagerComponent.h"
#include "Systems/CYTrapManagerComponent.h"
#include "Item/CYItemBase.h"
#include "PlayerCharacter.generated.h"

class UCYEffectManagerComponent;

UCLASS()
class CATCHME_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

protected:
    // 카메라 컴포넌트들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent;

    // 아이템/무기 시스템 컴포넌트들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    UCYEffectManagerComponent* EffectManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    UCYInventoryComponent* Inventory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    UCYWeaponManagerComponent* WeaponManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    UCYTrapManagerComponent* TrapManager;

    // 인터랙션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    ACYItemBase* NearbyItem;

    // 마우스 감도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float MouseSensitivity = 1.0f;

    // 효과 처리 함수들
    UFUNCTION()
    void OnEffectApplied(FGameplayTag EffectType, float Duration, float Magnitude);
    
    UFUNCTION()
    void OnEffectRemoved(FGameplayTag EffectType);
    
    // 이동 가능 여부
    UPROPERTY(BlueprintReadOnly, Category = "Player State")
    bool bCanMove = true;

public:
    virtual void Tick(float DeltaTime) override;

    // PlayerController에서 호출할 함수들
    void HandleMove(const FVector2D& Value);
    void HandleLook(const FVector2D& Value);
    void HandleJump();
    void HandleStopJump();
    void HandleInteract();
    void HandleUseItem();

protected:
    virtual void BeginPlay() override;

    // 아이템 상호작용
    void CheckForNearbyItems();
    void PickupItem(ACYItemBase* Item);

    // 라인 트레이스 유틸리티
    bool PerformLineTrace(FHitResult& HitResult, float Range = 1000.0f);
};