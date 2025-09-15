#include "Items/CYSlowTrap.h"
#include "Player/CYPlayerCharacter.h"
#include "GAS/CYGameplayEffects.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"

ACYSlowTrap::ACYSlowTrap()
{
    // 기본 정보 설정
    ItemName = FText::FromString("Slow Trap");
    ItemDescription = FText::FromString("Slows down enemies who step on it");
    TrapType = ETrapType::Slow;
    
    // 슬로우 트랩 설정
    TriggerRadius = 120.0f;
    SlowPercentage = 0.5f; // 50% 감소
    SlowDuration = 5.0f;
    SlowedMoveSpeed = 100.0f;

    // 트랩 데이터 설정
    TrapData.TrapType = ETrapType::Slow;
    TrapData.TrapName = ItemName;
    TrapData.TrapDescription = ItemDescription;
    TrapData.TriggerRadius = TriggerRadius;
    TrapData.TrapColor = FLinearColor::Blue; // 파란색으로 구분

    // 슬로우 효과 설정
    TrapData.GameplayEffects.Empty();
    TrapData.GameplayEffects.Add(UGE_SlowTrap::StaticClass());

    // 시각적 설정
    if (ItemMesh)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> SlowTrapMesh(TEXT("/Engine/BasicShapes/Cylinder"));
        if (SlowTrapMesh.Succeeded())
        {
            ItemMesh->SetStaticMesh(SlowTrapMesh.Object);
            ItemMesh->SetWorldScale3D(FVector(0.6f, 0.6f, 0.1f));
        }
    }
}

void ACYSlowTrap::OnTrapSpawned_Implementation()
{
    Super::OnTrapSpawned_Implementation();
    
    UE_LOG(LogTemp, Warning, TEXT("🧊 Slow Trap spawned with %f%% speed reduction"), SlowPercentage * 100);
}

void ACYSlowTrap::OnTrapArmed_Implementation()
{
    Super::OnTrapArmed_Implementation();
    
    UE_LOG(LogTemp, Warning, TEXT("🧊 Slow Trap Armed: %f%% speed reduction for %f seconds"), 
           SlowPercentage * 100, SlowDuration);
    
    // 슬로우 트랩 특유의 시각적 효과
    ShowSlowVisualEffect();
}

void ACYSlowTrap::OnTrapTriggered_Implementation(ACYPlayerCharacter* Target)
{
    Super::OnTrapTriggered_Implementation(Target);
    
    if (!Target) return;
    
    UE_LOG(LogTemp, Warning, TEXT("🧊 Slow Trap triggered on %s"), *Target->GetName());
    
    // 화면에 메시지 표시
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
            FString::Printf(TEXT("%s slowed by %d%%"), 
                          *Target->GetName(), 
                          (int32)(SlowPercentage * 100)));
    }
}

void ACYSlowTrap::SetupTrapVisuals_Implementation()
{
    Super::SetupTrapVisuals_Implementation();
    
    // 슬로우 트랩만의 추가 시각적 설정
    if (ItemMesh)
    {
        // 파란색 머티리얼 적용 (슬로우 효과 표시)
        UMaterialInterface* Material = ItemMesh->GetMaterial(0);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Blue);
                DynamicMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.8f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.2f);
                ItemMesh->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void ACYSlowTrap::PlayTrapSound_Implementation()
{
    Super::PlayTrapSound_Implementation();
    
    // 슬로우 트랩만의 사운드 (얼음 소리 등)
    // TODO: 슬로우 트랩 전용 사운드 추가
}

void ACYSlowTrap::ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target)
{
    Super::ApplyCustomEffects_Implementation(Target);
    
    if (!Target) return;
    
    // 슬로우 트랩만의 추가 효과
    ApplySlowEffect(Target);
    
    UE_LOG(LogTemp, Warning, TEXT("🧊 Applied slow trap custom effects to %s"), *Target->GetName());
}

void ACYSlowTrap::ApplySlowEffect(ACYPlayerCharacter* Target)
{
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    // 커스텀 슬로우 효과를 위한 추가 로직
    // 예: 특별한 슬로우 효과, 시각적 효과 등
    
    UE_LOG(LogTemp, Log, TEXT("🧊 Applying custom slow effect: %f speed for %f seconds"), 
           SlowedMoveSpeed, SlowDuration);
}

void ACYSlowTrap::ShowSlowVisualEffect()
{
    // 슬로우 트랩 활성화 시 시각적 효과
    // 예: 파란색 글로우, 얼음 파티클 등
    
    if (ItemMesh)
    {
        // 활성화 시 크기 약간 증가
        ItemMesh->SetWorldScale3D(FVector(0.7f, 0.7f, 0.12f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("🧊 Slow trap visual effects activated"));
}