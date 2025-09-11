#include "Components/CYWeaponComponent.h"
#include "Items/CYWeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "Components/CYInventoryComponent.h"
#include "CYGameplayTags.h"
#include "Net/UnrealNetwork.h"

UCYWeaponComponent::UCYWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UCYWeaponComponent, CurrentWeapon);
}

bool UCYWeaponComponent::EquipWeapon(ACYWeaponBase* Weapon)
{
    if (!Weapon || !GetOwner()->HasAuthority()) return false;

    UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: Equipping weapon %s"), *Weapon->ItemName.ToString());

    if (CurrentWeapon)
    {
        UnequipWeapon();
    }

    CurrentWeapon = Weapon;
    
    USkeletalMeshComponent* OwnerMesh = GetOwnerMesh();
    if (OwnerMesh)
    {
        Weapon->AttachToComponent(
            OwnerMesh,
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            WeaponSocketName
        );
    }

    if (Weapon->ItemMesh)
    {
        Weapon->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (Weapon->InteractionSphere)
    {
        Weapon->InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    OnWeaponChanged.Broadcast(nullptr, CurrentWeapon);
    UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: Weapon equipped successfully"));
    return true;
}

bool UCYWeaponComponent::UnequipWeapon()
{
    if (!CurrentWeapon || !GetOwner()->HasAuthority()) return false;

    ACYWeaponBase* OldWeapon = CurrentWeapon;
    
    CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CurrentWeapon = nullptr;

    OnWeaponChanged.Broadcast(OldWeapon, nullptr);
    return true;
}

bool UCYWeaponComponent::PerformAttack()
{
    UE_LOG(LogTemp, Warning, TEXT("=== UCYWeaponComponent::PerformAttack called ==="));
    
    // ✅ 인벤토리 상태는 무조건 출력
    if (UCYInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UCYInventoryComponent>())
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: Found InventoryComponent, printing status"));
        InventoryComp->PrintInventoryStatus();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: InventoryComponent not found!"));
    }

    if (!CurrentWeapon) 
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: 무기가 장착되지 않음"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: Current weapon: %s"), *CurrentWeapon->ItemName.ToString());

    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC) 
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: No ASC found"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: ASC found, trying to cast to CY ASC"));

    if (UCYAbilitySystemComponent* CYasc = Cast<UCYAbilitySystemComponent>(ASC))
    {
        const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
        UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: Trying to activate ability with tag: %s"), 
               *GameplayTags.Ability_Weapon_Attack.ToString());
        
        bool bResult = CYasc->TryActivateAbilityByTag(GameplayTags.Ability_Weapon_Attack);
        UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: Ability activation result: %s"), 
               bResult ? TEXT("SUCCESS") : TEXT("FAILED"));
        return bResult;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: Failed to cast ASC to CYAbilitySystemComponent"));
    }

    return false;
}

bool UCYWeaponComponent::PerformLineTrace(FHitResult& OutHit, float Range)
{
    UCameraComponent* Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
    if (!Camera) return false;

    FVector Start = Camera->GetComponentLocation();
    FVector End = Start + (Camera->GetForwardVector() * Range);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    return GetWorld()->LineTraceSingleByChannel(
        OutHit, Start, End, ECC_Visibility, Params
    );
}

void UCYWeaponComponent::OnRep_CurrentWeapon()
{
    OnWeaponChanged.Broadcast(nullptr, CurrentWeapon);
    
    if (CurrentWeapon)
    {
        USkeletalMeshComponent* OwnerMesh = GetOwnerMesh();
        if (OwnerMesh)
        {
            CurrentWeapon->AttachToComponent(
                OwnerMesh,
                FAttachmentTransformRules::SnapToTargetIncludingScale,
                WeaponSocketName
            );
        }
    }
}

UAbilitySystemComponent* UCYWeaponComponent::GetOwnerASC() const
{
    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
    {
        return ASI->GetAbilitySystemComponent();
    }
    return nullptr;
}

USkeletalMeshComponent* UCYWeaponComponent::GetOwnerMesh() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetMesh();
    }
    return nullptr;
}