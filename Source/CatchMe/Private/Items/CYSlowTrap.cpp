// CYSlowTrap.cpp - 고유 메쉬 및 효과 설정
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

    // ✅ TrapData 완전 설정
    TrapData.TrapType = ETrapType::Slow;
    TrapData.TrapName = ItemName;
    TrapData.TrapDescription = ItemDescription;
    TrapData.TriggerRadius = TriggerRadius;
    TrapData.TrapColor = FLinearColor::Blue; // 파란색
    
    // ✅ 슬로우 트랩 전용 메쉬/사운드/이펙트 (블루프린트에서 설정할 것)
    // TrapData.TrapMesh = nullptr; // 블루프린트에서 설정
    // TrapData.TriggerSound = nullptr; // 블루프린트에서 설정
    // TrapData.TriggerEffect = nullptr; // 블루프린트에서 설정

    // 슬로우 효과 설정
    TrapData.GameplayEffects.Empty();
    TrapData.GameplayEffects.Add(UGE_SlowTrap::StaticClass());

    UE_LOG(LogTemp, Warning, TEXT("✅ SlowTrap constructor completed"));
    
    // ✅ 생성자에서 기본 메쉬 설정 (안전함)
    if (ItemMesh)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> SlowTrapMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
        if (SlowTrapMesh.Succeeded())
        {
            ItemMesh->SetStaticMesh(SlowTrapMesh.Object);
            ItemMesh->SetWorldScale3D(FVector(0.8f, 0.8f, 0.1f));
            UE_LOG(LogTemp, Warning, TEXT("🎨 SlowTrap: Set default cylinder mesh in constructor"));
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
    
    // ✅ 슬로우 트랩 전용 트리거 이펙트 재생
    PlaySlowTriggerEffect();
}

void ACYSlowTrap::SetupTrapVisuals_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("🎨 SlowTrap::SetupTrapVisuals called"));

    if (ItemMesh)
    {
        // ✅ TrapData에서 메쉬 사용 (블루프린트에서 설정된 것)
        if (TrapData.TrapMesh)
        {
            ItemMesh->SetStaticMesh(TrapData.TrapMesh);
            ItemMesh->SetWorldScale3D(FVector(0.8f, 0.8f, 0.1f)); // 커스텀 메쉬용 스케일
            UE_LOG(LogTemp, Warning, TEXT("🎨 SlowTrap: Using TrapData mesh: %s"), *TrapData.TrapMesh->GetName());
        }
        else
        {
            // ✅ 생성자에서 설정된 메쉬 사용 (이미 안전하게 로드됨)
            UE_LOG(LogTemp, Warning, TEXT("🎨 SlowTrap: Using constructor-set mesh"));
        }
        
        // ✅ 슬로우 트랩 전용 스케일 보장 (넓고 얇은 원판)
        ItemMesh->SetWorldScale3D(FVector(0.8f, 0.8f, 0.1f));
        
        // ✅ 슬로우 전용 머티리얼 설정 (파란색, 약간 투명)
        UMaterialInterface* Material = ItemMesh->GetMaterial(0);
        if (Material && !Material->IsA<UMaterialInstanceDynamic>())
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TrapData.TrapColor);
                DynamicMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.3f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.7f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), 0.2f); // 약한 발광
                DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.8f); // 약간 투명
                ItemMesh->SetMaterial(0, DynamicMaterial);
                
                UE_LOG(LogTemp, Warning, TEXT("🎨 SlowTrap: Applied blue semi-transparent material"));
            }
        }
        
        // ✅ 가시성 강제 보장
        ItemMesh->SetVisibility(true);
        ItemMesh->SetHiddenInGame(false);
        ItemMesh->MarkRenderStateDirty();
        
        UE_LOG(LogTemp, Warning, TEXT("🎨 SlowTrap visuals setup complete"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ SlowTrap: ItemMesh is NULL"));
    }
}

void ACYSlowTrap::PlayTrapSound_Implementation()
{
    // ✅ TrapData에서 사운드 재생
    if (TrapData.TriggerSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TrapData.TriggerSound, GetActorLocation());
        UE_LOG(LogTemp, Log, TEXT("🧊 Played SlowTrap trigger sound"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("🧊 SlowTrap: No trigger sound set in TrapData"));
    }
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
    UE_LOG(LogTemp, Log, TEXT("🧊 Applying custom slow effect: %f speed for %f seconds"), 
           SlowedMoveSpeed, SlowDuration);
}

void ACYSlowTrap::ShowSlowVisualEffect()
{
    // 슬로우 트랩 활성화 시 시각적 효과
    if (ItemMesh)
    {
        // 활성화 시 크기 약간 증가 (파란색 원판 확장)
        ItemMesh->SetWorldScale3D(FVector(0.9f, 0.9f, 0.12f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("🧊 Slow trap visual effects activated"));
}

void ACYSlowTrap::PlaySlowTriggerEffect()
{
    // 트리거 시 전용 효과 (사운드 + 이펙트)
    PlayTrapSound();
    
    // ✅ TrapData에서 이펙트 재생
    if (TrapData.TriggerEffect)
    {
        FVector EffectLocation = GetActorLocation();
        EffectLocation.Z += 20.0f; // 슬로우는 지면 근처에
        
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrapData.TriggerEffect, EffectLocation);
        UE_LOG(LogTemp, Log, TEXT("🧊 Spawned slow effect from TrapData"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("🧊 SlowTrap: No trigger effect set in TrapData"));
    }
    
    // 추가적인 슬로우 전용 효과
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
            TEXT("🌀 SLOW ACTIVATED! 🌀"));
    }
}