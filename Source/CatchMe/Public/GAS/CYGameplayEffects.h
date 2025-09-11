#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CYGameplayEffects.generated.h"

// 트랩 이동 제한 효과
UCLASS()
class CATCHME_API UGE_ImmobilizeTrap : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_ImmobilizeTrap();
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