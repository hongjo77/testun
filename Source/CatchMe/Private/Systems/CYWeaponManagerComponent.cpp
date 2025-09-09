// Systems/CYWeaponManagerComponent.cpp
#include "Systems/CYWeaponManagerComponent.h"
#include "Weapon/CYWeaponBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

UCYWeaponManagerComponent::UCYWeaponManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UCYWeaponManagerComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCYWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UCYWeaponManagerComponent, CurrentWeapon);
}

void UCYWeaponManagerComponent::ServerEquipWeapon_Implementation(ACYWeaponBase* Weapon)
{
    if (!GetOwner()->HasAuthority() || !Weapon) return;

    // 기존 무기 해제
    if (CurrentWeapon)
    {
        ServerUnequipWeapon();
    }

    // 새 무기 장착
    CurrentWeapon = Weapon;
    AttachWeaponToCharacter(Weapon);
    OnWeaponEquipped.Broadcast(Weapon);
    
    UE_LOG(LogTemp, Warning, TEXT("Equipped weapon: %s"), *Weapon->GetName());
}

void UCYWeaponManagerComponent::ServerUseWeapon_Implementation(FVector HitLocation, AActor* HitTarget)
{
    if (!GetOwner()->HasAuthority() || !CurrentWeapon) return;

    // 무기 사용
    CurrentWeapon->ServerAttack(HitLocation, HitTarget);
}

void UCYWeaponManagerComponent::ServerUnequipWeapon_Implementation()
{
    if (!GetOwner()->HasAuthority()) return;

    if (CurrentWeapon)
    {
        DetachWeaponFromCharacter();
        CurrentWeapon = nullptr;
        OnWeaponUnequipped.Broadcast();
        
        UE_LOG(LogTemp, Warning, TEXT("Unequipped weapon"));
    }
}

void UCYWeaponManagerComponent::AttachWeaponToCharacter(ACYWeaponBase* Weapon)
{
    if (!Weapon) return;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->GetMesh()) return;

    // 무기를 캐릭터 손에 부착
    Weapon->AttachToComponent(
        Character->GetMesh(),
        FAttachmentTransformRules::SnapToTargetIncludingScale,
        WeaponSocketName
    );

    // 무기 소유자 설정
    Weapon->SetOwner(Character);
}

void UCYWeaponManagerComponent::DetachWeaponFromCharacter()
{
    if (!CurrentWeapon) return;

    // 무기 분리
    CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CurrentWeapon->SetOwner(nullptr);
}