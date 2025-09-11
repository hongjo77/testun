#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CYGameplayEffects.generated.h"

UCLASS()
class CATCHME_API UGE_MovementModifier : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_MovementModifier();
};

// 기존 효과들은 그대로 유지
UCLASS()
class CATCHME_API UGE_ImmobilizeTrap : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_ImmobilizeTrap();
};

UCLASS()
class CATCHME_API UGE_SlowTrap : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_SlowTrap();
};

// 무기 데미지 효과
UCLASS()
class CATCHME_API UGE_WeaponDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_WeaponDamage();
};

// 무기 공격 쿨다운
UCLASS()
class CATCHME_API UGE_WeaponAttackCooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_WeaponAttackCooldown();
};

// 트랩 배치 쿨다운
UCLASS()
class CATCHME_API UGE_TrapPlaceCooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_TrapPlaceCooldown();
};

// 초기 스탯 설정
UCLASS()
class CATCHME_API UGE_InitialStats : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_InitialStats();
};