#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CYTrapData.generated.h"

class UGameplayEffect;
class UStaticMesh;
class USoundBase;
class UParticleSystem;

UENUM(BlueprintType)
enum class ETrapType : uint8
{
    Slow        UMETA(DisplayName = "Slow Trap"),
    Freeze      UMETA(DisplayName = "Freeze Trap"),
    Damage      UMETA(DisplayName = "Damage Trap"),
    Explosion   UMETA(DisplayName = "Explosion Trap")
};

USTRUCT(BlueprintType)
struct CATCHME_API FTrapData : public FTableRowBase
{
    GENERATED_BODY()

    FTrapData()
    {
        TrapType = ETrapType::Slow;
        TriggerRadius = 100.0f;
        ArmingDelay = 2.0f;
        TrapLifetime = 60.0f;
        TrapMesh = nullptr;
        TriggerSound = nullptr;
        TriggerEffect = nullptr;
    }

    // 트랩 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
    ETrapType TrapType;

    // 트랩 이름 및 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
    FText TrapName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
    FText TrapDescription;

    // 게임플레이 효과들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

    // 트랩 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ArmingDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TrapLifetime;

    // 시각적/오디오 효과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    UStaticMesh* TrapMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* TriggerSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* TriggerEffect;

    // 색상 (구분용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    FLinearColor TrapColor = FLinearColor::White;
};