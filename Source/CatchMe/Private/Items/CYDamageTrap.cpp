// CYDamageTrap.cpp - 고유 메쉬 및 효과 설정
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

    // ✅ TrapData 완전 설정
    TrapData.TrapType = ETrapType::Damage;
    TrapData.TrapName = ItemName;
    TrapData.TrapDescription = ItemDescription;
    TrapData.TriggerRadius = TriggerRadius;
    TrapData.TrapColor = FLinearColor::Red; // 빨간색
    
    // ✅ 데미지 트랩 전용 메쉬/사운드/이펙트 (블루프린트에서 설정할 것)
    // TrapData.TrapMesh = nullptr; // 블루프린트에서 설정
    // TrapData.TriggerSound = nullptr; // 블루프린트에서 설정
    // TrapData.TriggerEffect = nullptr; // 블루프린트에서 설정

    // 데미지 효과 설정
    TrapData.GameplayEffects.Empty();
    TrapData.GameplayEffects.Add(UGE_WeaponDamage::StaticClass());

    UE_LOG(LogTemp, Warning, TEXT("✅ DamageTrap constructor completed"));
    
    // ✅ 생성자에서 기본 메쉬 설정 (안전함)
    if (ItemMesh)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> DamageTrapMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
        if (DamageTrapMesh.Succeeded())
        {
            ItemMesh->SetStaticMesh(DamageTrapMesh.Object);
            ItemMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.8f));
            UE_LOG(LogTemp, Warning, TEXT("🎨 DamageTrap: Set default cone mesh in constructor"));
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
    
    // ✅ 데미지 트랩 전용 트리거 이펙트 재생
    PlayDamageTriggerEffect();
}

void ACYDamageTrap::SetupTrapVisuals_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("🎨 DamageTrap::SetupTrapVisuals called"));

    if (ItemMesh)
    {
        // ✅ TrapData에서 메쉬 사용 (블루프린트에서 설정된 것)
        if (TrapData.TrapMesh)
        {
            ItemMesh->SetStaticMesh(TrapData.TrapMesh);
            ItemMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.8f)); // 커스텀 메쉬용 스케일
            UE_LOG(LogTemp, Warning, TEXT("🎨 DamageTrap: Using TrapData mesh: %s"), *TrapData.TrapMesh->GetName());
        }
        else
        {
            // ✅ 생성자에서 설정된 메쉬 사용 (이미 안전하게 로드됨)
            UE_LOG(LogTemp, Warning, TEXT("🎨 DamageTrap: Using constructor-set mesh"));
        }
        
        // ✅ 데미지 트랩 전용 스케일 보장 (뾰족하고 위험해 보이게)
        ItemMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.8f));
        
        // ✅ 데미지 전용 머티리얼 설정 (빨간색, 금속성)
        UMaterialInterface* Material = ItemMesh->GetMaterial(0);
        if (Material && !Material->IsA<UMaterialInstanceDynamic>())
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TrapData.TrapColor);
                DynamicMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.8f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.2f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), 0.6f); // 강한 발광
                ItemMesh->SetMaterial(0, DynamicMaterial);
                
                UE_LOG(LogTemp, Warning, TEXT("🎨 DamageTrap: Applied red metallic material"));
            }
        }
        
        // ✅ 가시성 강제 보장
        ItemMesh->SetVisibility(true);
        ItemMesh->SetHiddenInGame(false);
        ItemMesh->MarkRenderStateDirty();
        
        UE_LOG(LogTemp, Warning, TEXT("🎨 DamageTrap visuals setup complete"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ DamageTrap: ItemMesh is NULL"));
    }
}

void ACYDamageTrap::PlayTrapSound_Implementation()
{
    // ✅ TrapData에서 사운드 재생
    if (TrapData.TriggerSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TrapData.TriggerSound, GetActorLocation());
        UE_LOG(LogTemp, Log, TEXT("🗡️ Played DamageTrap trigger sound"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("🗡️ DamageTrap: No trigger sound set in TrapData"));
    }
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
}

void ACYDamageTrap::ApplyDamageOverTime(ACYPlayerCharacter* Target)
{
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    // DoT(Damage over Time) 효과
    UE_LOG(LogTemp, Warning, TEXT("🗡️ Applying DoT: %f damage every %f seconds for %d ticks"), 
           DamageAmount / DamageOverTimeTicks, DamageOverTimeInterval, DamageOverTimeTicks);
}

void ACYDamageTrap::ShowDamageVisualEffect()
{
    // 데미지 트랩 활성화 시 시각적 효과
    if (ItemMesh)
    {
        // 활성화 시 모양 변경 (더 날카롭게)
        ItemMesh->SetWorldScale3D(FVector(0.6f, 0.6f, 0.9f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("🗡️ Damage trap visual effects activated"));
}

void ACYDamageTrap::CreateSpikeEffect()
{
    // ✅ TrapData에서 이펙트 재생
    if (TrapData.TriggerEffect)
    {
        FVector EffectLocation = GetActorLocation();
        EffectLocation.Z += 30.0f; // 약간 위에 효과 생성
        
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrapData.TriggerEffect, EffectLocation);
        UE_LOG(LogTemp, Log, TEXT("🗡️ Spawned spike effect from TrapData"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("🗡️ DamageTrap: No trigger effect set in TrapData"));
    }
}

void ACYDamageTrap::PlayDamageTriggerEffect()
{
    // 트리거 시 전용 효과 (사운드 + 이펙트)
    PlayTrapSound();
    CreateSpikeEffect();
    
    // 추가적인 데미지 전용 효과
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
            TEXT("⚔️ DAMAGE ACTIVATED! ⚔️"));
    }
}