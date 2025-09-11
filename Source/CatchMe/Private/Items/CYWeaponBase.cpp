#include "Items/CYWeaponBase.h"

#include "CYGameplayTags.h"
#include "Player/CYPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

ACYWeaponBase::ACYWeaponBase()
{
    // 기본 무기 설정
    ItemName = FText::FromString("Base Weapon");
    ItemDescription = FText::FromString("A basic weapon");
    // 무기 태그 설정
    const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
    ItemTag = GameplayTags.Item_Weapon;
}

void ACYWeaponBase::OnPickup(ACYPlayerCharacter* Character)
{
    // 부모 클래스의 OnPickup 호출 (어빌리티 부여)
    Super::OnPickup(Character);

    // 무기 장착
    Equip(Character);
}

void ACYWeaponBase::Equip(ACYPlayerCharacter* Character)
{
    if (!Character) return;

    OwningCharacter = Character;
    
    // 무기를 캐릭터에 부착
    if (Character->GetMesh())
    {
        AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            TEXT("hand_r")  // 오른손 소켓
        );
    }

    // 충돌 비활성화
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Warning, TEXT("Weapon equipped: %s"), *ItemName.ToString());
}

void ACYWeaponBase::Unequip()
{
    if (!OwningCharacter) return;

    // 부착 해제
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    
    OwningCharacter = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("Weapon unequipped: %s"), *ItemName.ToString());
}