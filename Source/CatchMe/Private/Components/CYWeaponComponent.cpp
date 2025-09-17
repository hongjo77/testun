// CYWeaponComponent.cpp - GameplayTags 중복 초기화 문제 해결
#include "Components/CYWeaponComponent.h"
#include "Items/CYWeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
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

    if (CurrentWeapon)
    {
        UnequipWeapon();
    }

    CurrentWeapon = Weapon;
    AttachWeaponToOwner(Weapon);
    DisableWeaponInteraction(Weapon);
    
    OnWeaponChanged.Broadcast(nullptr, CurrentWeapon);
    UE_LOG(LogTemp, Log, TEXT("Weapon equipped: %s"), *Weapon->ItemName.ToString());
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
    UE_LOG(LogTemp, Warning, TEXT("🗡️ PerformAttack called - HasAuthority: %s"), 
           GetOwner()->HasAuthority() ? TEXT("true") : TEXT("false"));
    
    // 서버에서만 실행
    if (!GetOwner()->HasAuthority()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ PerformAttack: Not authority, returning false"));
        return false;
    }

    // 무기가 있으면 공격만 수행
    if (CurrentWeapon) 
    {
        UE_LOG(LogTemp, Warning, TEXT("🗡️ PerformAttack: CurrentWeapon found: %s"), 
               *CurrentWeapon->ItemName.ToString());
        return ExecuteWeaponAttack();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ PerformAttack: No CurrentWeapon equipped"));
    }
    
    return false;
}

bool UCYWeaponComponent::ExecuteWeaponAttack()
{
    UE_LOG(LogTemp, Warning, TEXT("🗡️ ExecuteWeaponAttack called"));
    
    if (!CurrentWeapon) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ExecuteWeaponAttack: CurrentWeapon is null"));
        return false;
    }

    UCYAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
    if (!ASC) 
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ExecuteWeaponAttack: AbilitySystemComponent is null"));
        return false;
    }

    // ✅ GameplayTags 초기화 제거 - 한 번만 초기화되도록 함
    // FCYGameplayTags::InitializeNativeTags(); // 이 줄 제거!
    
    // ✅ 안전한 태그 가져오기
    FGameplayTag WeaponAttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Attack"));
    
    // ✅ 태그 유효성 검사 강화
    if (!WeaponAttackTag.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Weapon attack tag is invalid! Tag: %s"), *WeaponAttackTag.ToString());
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Using tag: %s"), *WeaponAttackTag.ToString());
    
    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(WeaponAttackTag);
    
    TArray<FGameplayAbilitySpec*> ActivatableAbilities;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, ActivatableAbilities);
    
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Found %d activatable abilities"), ActivatableAbilities.Num());
    
    TArray<FGameplayAbilitySpec> AllAbilities = ASC->GetActivatableAbilities();
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Total abilities in ASC: %d"), AllAbilities.Num());
    
    bool bResult = ASC->TryActivateAbilityByTag(WeaponAttackTag);
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Weapon attack result: %s"), bResult ? TEXT("Success") : TEXT("Failed"));
    
    return bResult;
}

void UCYWeaponComponent::DisplayInventoryStatus()
{
    // ✅ 클라이언트에서 직접 호출 가능한 일반 함수
    if (!GEngine) return;

    UCYInventoryComponent* InventoryComp = GetOwner()->FindComponentByClass<UCYInventoryComponent>();
    if (!InventoryComp)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("❌ No InventoryComponent found"));
        return;
    }

    // 기존 메시지 제거
    GEngine->ClearOnScreenDebugMessages();

    // 인벤토리 상태 표시
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("=== 📦 INVENTORY STATUS ==="));
    
    // 무기 슬롯 (1~3번 키)
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("🗡️ WEAPONS (Keys 1-3):"));
    for (int32 i = 0; i < InventoryComp->WeaponSlots.Num(); ++i)
    {
        FString WeaponInfo;
        if (InventoryComp->WeaponSlots[i])
        {
            WeaponInfo = FString::Printf(TEXT("  [%d] %s x%d"), 
                i + 1, 
                *InventoryComp->WeaponSlots[i]->ItemName.ToString(), 
                InventoryComp->WeaponSlots[i]->ItemCount
            );
            
            if (CurrentWeapon == InventoryComp->WeaponSlots[i])
            {
                WeaponInfo += TEXT(" ⭐ EQUIPPED");
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, WeaponInfo);
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, WeaponInfo);
            }
        }
        else
        {
            WeaponInfo = FString::Printf(TEXT("  [%d] Empty"), i + 1);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, WeaponInfo);
        }
    }
    
    // 아이템 슬롯 (4~9번 키)
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("🎒 ITEMS (Keys 4-9):"));
    int32 MaxDisplayItems = FMath::Min(6, InventoryComp->ItemSlots.Num());
    for (int32 i = 0; i < MaxDisplayItems; ++i)
    {
        FString ItemInfo;
        if (InventoryComp->ItemSlots[i])
        {
            ItemInfo = FString::Printf(TEXT("  [%d] %s x%d"), 
                i + 4, 
                *InventoryComp->ItemSlots[i]->ItemName.ToString(), 
                InventoryComp->ItemSlots[i]->ItemCount
            );
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, ItemInfo);
        }
        else
        {
            ItemInfo = FString::Printf(TEXT("  [%d] Empty"), i + 4);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, ItemInfo);
        }
    }
    
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("=================="));
}

void UCYWeaponComponent::ClientDisplayInventoryStatus_Implementation()
{
    // ✅ RPC 버전은 일반 함수 호출
    DisplayInventoryStatus();
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
        AttachWeaponToOwner(CurrentWeapon);
    }
}

UCYAbilitySystemComponent* UCYWeaponComponent::GetOwnerAbilitySystemComponent() const
{
    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
    {
        return Cast<UCYAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
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

void UCYWeaponComponent::AttachWeaponToOwner(ACYWeaponBase* Weapon)
{
    if (!Weapon) return;

    USkeletalMeshComponent* OwnerMesh = GetOwnerMesh();
    if (OwnerMesh)
    {
        Weapon->AttachToComponent(
            OwnerMesh,
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            WeaponSocketName
        );
    }
}

void UCYWeaponComponent::DisableWeaponInteraction(ACYWeaponBase* Weapon)
{
    if (!Weapon) return;

    if (Weapon->ItemMesh)
    {
        Weapon->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (Weapon->InteractionSphere)
    {
        Weapon->InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}