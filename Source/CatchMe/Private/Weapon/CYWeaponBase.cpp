// Weapon/CYWeaponBase.cpp
#include "Weapon/CYWeaponBase.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"

ACYWeaponBase::ACYWeaponBase()
{
    // 무기 기본 설정
    ItemName = FText::FromString("Base Weapon");
    ItemDescription = FText::FromString("A basic weapon");
    
    BaseDamage = 50.0f;
    AttackRange = 200.0f;
    AttackCooldown = 1.0f;
    LastAttackTime = 0.0f;
}

void ACYWeaponBase::ServerAttack_Implementation(FVector HitLocation, AActor* HitTarget)
{
    if (!HasAuthority()) return;

    // 쿨다운 체크
    if (!CanAttack()) return;

    // 쿨다운 업데이트
    LastAttackTime = GetWorld()->GetTimeSeconds();

    // 무기별 공격 로직 실행
    ExecuteAttack(HitLocation, HitTarget);

    // 공격 이펙트 재생
    MulticastPlayAttackEffect(HitLocation);

    UE_LOG(LogTemp, Warning, TEXT("%s attacked at %s"), *GetName(), *HitLocation.ToString());
}

void ACYWeaponBase::MulticastPlayAttackEffect_Implementation(FVector HitLocation)
{
    // 블루프린트에서 구현할 이펙트 재생
    // 파티클, 사운드, 애니메이션 등
}

bool ACYWeaponBase::CanAttack()
{
    if (!GetWorld()) return false;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

float ACYWeaponBase::GetRemainingCooldown()
{
    if (!GetWorld()) return 0.0f;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeElapsed = CurrentTime - LastAttackTime;
    return FMath::Max(0.0f, AttackCooldown - TimeElapsed);
}

void ACYWeaponBase::ExecuteAttack(FVector HitLocation, AActor* HitTarget)
{
    // 타겟이 있으면 데미지 적용
    if (HitTarget)
    {
        ApplyDamage(HitTarget, BaseDamage);
        ApplyWeaponEffect(HitTarget);
    }
}

void ACYWeaponBase::ApplyWeaponEffect(AActor* HitTarget)
{
    // 기본 구현은 비어있음 - 자식 클래스에서 오버라이드
}

void ACYWeaponBase::ApplyDamage(AActor* Target, float Damage)
{
    if (!Target) return;

    // 데미지 이벤트 생성
    FPointDamageEvent DamageEvent;
    DamageEvent.Damage = Damage;
    DamageEvent.HitInfo = FHitResult();

    // 데미지 적용
    Target->TakeDamage(Damage, DamageEvent, GetInstigator()->GetController(), this);
}