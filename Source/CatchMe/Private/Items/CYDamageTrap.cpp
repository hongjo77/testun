#include "Items/CYDamageTrap.h"
#include "Player/CYPlayerCharacter.h"
#include "GAS/CYGameplayEffects.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"

ACYDamageTrap::ACYDamageTrap()
{
    // 기본 정보 설정
    ItemName = FText::FromString("Damage Trap");
    ItemDescription = FText::FromString("Deals direct damage to enemies");
    TrapType = ETrapType::Damage;
    
    // 데미지 트랩 설정
    TriggerRadius = 90.0f;
    DamageAmount = 75.0f;
    bInstantDamage = true;
    DamageOverTimeInterval = 1.0f;
    DamageOverTimeTicks = 3;

    // 트랩 데이터 설정
    TrapData.TrapType = ETrapType::Damage;
    TrapData.TrapName = ItemName;
    TrapData.TrapDescription = ItemDescription;
    TrapData.TriggerRadius = TriggerRadius;
    TrapData.TrapColor = FLinearColor::Red; // 빨간색으로 구분

    // 데미지 효과 설정
    TrapData.GameplayEffects.Empty();
    TrapData.GameplayEffects.Add(UGE_WeaponDamage::StaticClass()); // 기존 데미지 효과 재사용

    // 시각적 설정
    if (ItemMesh)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> DamageTrapMesh(TEXT("/Engine/BasicShapes/Cylinder"));
        if (DamageTrapMesh.Succeeded())
        {
            ItemMesh->SetStaticMesh(DamageTrapMesh.Object);
            ItemMesh->SetWorldScale3D(FVector(0.4f, 0.4f, 0.2f)); // 더 작고 높게
        }
    }
}

void ACYDamageTrap::OnTrapSpawned_Implementation()
{
    Super::OnTrapSpawned_Implementation();
    
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Damage Trap spawned with %f damage"), DamageAmount);
}

void ACYDamageTrap::OnTrapArmed_Implementation()
{
    Super::OnTrapArmed_Implementation();
    
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Damage Trap Armed: %f damage"), DamageAmount);
    
    // 데미지 트랩 특유의 시각적 효과
    ShowDamageVisualEffect();
}

void ACYDamageTrap::OnTrapTriggered_Implementation(ACYPlayerCharacter* Target)
{
    Super::OnTrapTriggered_Implementation(Target);
    
    if (!Target) return;
    
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Damage Trap triggered on %s"), *Target->GetName());
    
    // 화면에 메시지 표시
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("%s took %f damage!"), 
                          *Target->GetName(), 
                          DamageAmount));
    }
}

void ACYDamageTrap::SetupTrapVisuals_Implementation()
{
    Super::SetupTrapVisuals_Implementation();
    
    // 데미지 트랩만의 추가 시각적 설정
    if (ItemMesh)
    {
        // 위험한 빨간색 머티리얼 적용
        UMaterialInterface* Material = ItemMesh->GetMaterial(0);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Red);
                DynamicMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.7f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.3f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), 0.5f); // 강한 발광
                ItemMesh->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void ACYDamageTrap::PlayTrapSound_Implementation()
{
    Super::PlayTrapSound_Implementation();
    
    // 데미지 트랩만의 사운드 (금속 소리, 가시 소리 등)
    // TODO: 데미지 트랩 전용 사운드 추가
}

void ACYDamageTrap::ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target)
{
    Super::ApplyCustomEffects_Implementation(Target);
    
    if (!Target) return;
    
    // 데미지 트랩만의 추가 효과
    if (bInstantDamage)
    {
        ApplyInstantDamage(Target);
    }
    else
    {
        ApplyDamageOverTime(Target);
    }
    
    CreateSpikeEffect();
    
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Applied damage trap custom effects to %s"), *Target->GetName());
}

void ACYDamageTrap::ApplyInstantDamage(ACYPlayerCharacter* Target)
{
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    // 즉시 데미지 효과
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Applying instant damage: %f to %s"), 
           DamageAmount, *Target->GetName());
    
    // 여기서 추가적인 즉시 데미지 로직을 구현할 수 있음
    // 예: 크리티컬 히트, 특수 상태 이상 등
}

void ACYDamageTrap::ApplyDamageOverTime(ACYPlayerCharacter* Target)
{
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    // DoT(Damage over Time) 효과
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Applying DoT: %f damage every %f seconds for %d ticks"), 
           DamageAmount / DamageOverTimeTicks, DamageOverTimeInterval, DamageOverTimeTicks);
    
    // TODO: DoT 효과를 위한 별도의 GameplayEffect 클래스 생성 및 적용
}

void ACYDamageTrap::ShowDamageVisualEffect()
{
    // 데미지 트랩 활성화 시 시각적 효과
    // 예: 가시 돌출, 빨간 오라 등
    
    if (ItemMesh)
    {
        // 활성화 시 모양 변경 (더 날카롭게)
        ItemMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.25f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("🗡️ Damage trap visual effects activated"));
}

void ACYDamageTrap::CreateSpikeEffect()
{
    // 트랩 트리거 시 가시/스파이크 효과 생성
    // 예: 가시 파티클, 피 효과 등
    
    FVector EffectLocation = GetActorLocation();
    EffectLocation.Z += 30.0f; // 약간 위에 효과 생성
    
    // TODO: 가시/스파이크 파티클 시스템 추가
    // if (SpikeParticleSystem)
    // {
    //     UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SpikeParticleSystem, EffectLocation);
    // }
    
    UE_LOG(LogTemp, Log, TEXT("🗡️ Spike effect created at location %s"), *EffectLocation.ToString());
}