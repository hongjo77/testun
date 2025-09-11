#include "Components/CYWeaponComponent.h"
#include "Items/CYWeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
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

    // 기존 무기 해제
    if (CurrentWeapon)
    {
        UnequipWeapon();
    }

    CurrentWeapon = Weapon;
    
    // 무기 장착
    USkeletalMeshComponent* OwnerMesh = GetOwnerMesh();
    if (OwnerMesh)
    {
        Weapon->AttachToComponent(
            OwnerMesh,
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            WeaponSocketName
        );
    }

    // 충돌 비활성화
    if (Weapon->ItemMesh)
    {
        Weapon->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (Weapon->InteractionSphere)
    {
        Weapon->InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    OnWeaponChanged.Broadcast(nullptr, CurrentWeapon);
    return true;
}

bool UCYWeaponComponent::UnequipWeapon()
{
    if (!CurrentWeapon || !GetOwner()->HasAuthority()) return false;

    ACYWeaponBase* OldWeapon = CurrentWeapon;
    
    // 무기 분리
    CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CurrentWeapon = nullptr;

    OnWeaponChanged.Broadcast(OldWeapon, nullptr);
    return true;
}

bool UCYWeaponComponent::PerformAttack()
{
    if (!CurrentWeapon) return false;

    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC) return false;

    // 무기 공격 어빌리티 실행
    FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag("Ability.Weapon.Attack");
    return ASC->TryActivateAbilityByTag(AttackTag);
}

bool UCYWeaponComponent::PerformLineTrace(FHitResult& OutHit, float Range)
{
    // 카메라 컴포넌트 찾기
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
    
    // 클라이언트에서 무기 장착 처리
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