// CYFreezeTrap.cpp - 고유 메쉬 및 효과 설정
#include "Items/CYFreezeTrap.h"
#include "Player/CYPlayerCharacter.h"
#include "GAS/CYGameplayEffects.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"

ACYFreezeTrap::ACYFreezeTrap()
{
    // 기본 정보 설정
    ItemName = FText::FromString("Freeze Trap");
    ItemDescription = FText::FromString("Completely immobilizes enemies");
    TrapType = ETrapType::Freeze;
    
    // 프리즈 트랩 설정
    TriggerRadius = 100.0f;
    FreezeDuration = 3.0f;
    bDisableJumping = true;
    bDisableAbilities = true;

    // ✅ TrapData 완전 설정
    TrapData.TrapType = ETrapType::Freeze;
    TrapData.TrapName = ItemName;
    TrapData.TrapDescription = ItemDescription;
    TrapData.TriggerRadius = TriggerRadius;
    TrapData.TrapColor = FLinearColor::Green; // 청록색
    
    // ✅ 프리즈 트랩 전용 메쉬 설정 (블루프린트에서 설정할 것)
    // TrapData.TrapMesh = nullptr; // 블루프린트에서 설정
    
    // ✅ 사운드 및 이펙트 (블루프린트에서 설정할 것)
    // TrapData.TriggerSound = nullptr; // 블루프린트에서 설정
    // TrapData.TriggerEffect = nullptr; // 블루프린트에서 설정

    // 프리즈 효과 설정
    TrapData.GameplayEffects.Empty();
    TrapData.GameplayEffects.Add(UGE_ImmobilizeTrap::StaticClass());

    UE_LOG(LogTemp, Warning, TEXT("✅ FreezeTrap constructor completed"));
    
    // ✅ 생성자에서 기본 메쉬 설정 (안전함)
    if (ItemMesh)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> FreezeTrapMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (FreezeTrapMesh.Succeeded())
        {
            ItemMesh->SetStaticMesh(FreezeTrapMesh.Object);
            ItemMesh->SetWorldScale3D(FVector(0.8f, 0.8f, 0.3f));
            UE_LOG(LogTemp, Warning, TEXT("🎨 FreezeTrap: Set default cube mesh in constructor"));
        }
    }
}

void ACYFreezeTrap::OnTrapSpawned_Implementation()
{
    Super::OnTrapSpawned_Implementation();
    
    UE_LOG(LogTemp, Warning, TEXT("❄️ Freeze Trap spawned with %f seconds freeze duration"), FreezeDuration);
}

void ACYFreezeTrap::OnTrapArmed_Implementation()
{
    Super::OnTrapArmed_Implementation();
    
    UE_LOG(LogTemp, Warning, TEXT("❄️ Freeze Trap Armed: %f seconds immobilization"), FreezeDuration);
    
    // 프리즈 트랩 특유의 시각적 효과
    ShowFreezeVisualEffect();
}

void ACYFreezeTrap::OnTrapTriggered_Implementation(ACYPlayerCharacter* Target)
{
    Super::OnTrapTriggered_Implementation(Target);
    
    if (!Target) return;
    
    UE_LOG(LogTemp, Warning, TEXT("❄️ Freeze Trap triggered on %s"), *Target->GetName());
    
    // 화면에 메시지 표시
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("%s frozen for %f seconds"), 
                          *Target->GetName(), 
                          FreezeDuration));
    }
    
    // ✅ 프리즈 트랩 전용 트리거 이펙트 재생
    PlayFreezeTriggerEffect();
}

void ACYFreezeTrap::SetupTrapVisuals_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("🎨 FreezeTrap::SetupTrapVisuals called"));

    if (ItemMesh)
    {
        // ✅ TrapData에서 메쉬 사용 (블루프린트에서 설정된 것)
        if (TrapData.TrapMesh)
        {
            ItemMesh->SetStaticMesh(TrapData.TrapMesh);
            UE_LOG(LogTemp, Warning, TEXT("🎨 FreezeTrap: Using TrapData mesh: %s"), *TrapData.TrapMesh->GetName());
        }
        else if (!ItemMesh->GetStaticMesh())
        {
            // ✅ 백업: 런타임에 안전한 메쉬 로딩 (생성자에서 실패한 경우만)
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            if (CubeMesh)
            {
                ItemMesh->SetStaticMesh(CubeMesh);
                UE_LOG(LogTemp, Warning, TEXT("🎨 FreezeTrap: Using runtime loaded cube mesh"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ FreezeTrap: Failed to load cube mesh"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("🎨 FreezeTrap: Using constructor-set mesh"));
        }
        
        // ✅ 프리즈 트랩 전용 스케일 및 색상
        ItemMesh->SetWorldScale3D(FVector(0.8f, 0.8f, 0.3f)); // 큐브 모양
        
        // ✅ 프리즈 전용 머티리얼 설정
        UMaterialInterface* Material = ItemMesh->GetMaterial(0);
        if (Material && !Material->IsA<UMaterialInstanceDynamic>())
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TrapData.TrapColor);
                DynamicMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.9f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.1f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), 0.5f); // 강한 발광
                ItemMesh->SetMaterial(0, DynamicMaterial);
                
                UE_LOG(LogTemp, Warning, TEXT("🎨 FreezeTrap: Applied cyan glowing material"));
            }
        }
        
        // ✅ 가시성 강제 보장
        ItemMesh->SetVisibility(true);
        ItemMesh->SetHiddenInGame(false);
        ItemMesh->MarkRenderStateDirty();
        
        UE_LOG(LogTemp, Warning, TEXT("🎨 FreezeTrap visuals setup complete"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ FreezeTrap: ItemMesh is NULL"));
    }
}

void ACYFreezeTrap::PlayTrapSound_Implementation()
{
    // ✅ TrapData에서 사운드 재생
    if (TrapData.TriggerSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TrapData.TriggerSound, GetActorLocation());
        UE_LOG(LogTemp, Log, TEXT("❄️ Played FreezeTrap trigger sound"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❄️ FreezeTrap: No trigger sound set in TrapData"));
    }
}

void ACYFreezeTrap::ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target)
{
    Super::ApplyCustomEffects_Implementation(Target);
    
    if (!Target) return;
    
    // 프리즈 트랩만의 추가 효과
    ApplyFreezeEffect(Target);
    CreateIceEffect();
    
    UE_LOG(LogTemp, Warning, TEXT("❄️ Applied freeze trap custom effects to %s"), *Target->GetName());
}

void ACYFreezeTrap::ApplyFreezeEffect(ACYPlayerCharacter* Target)
{
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    // 프리즈 관련 추가 효과
    if (bDisableJumping)
    {
        UE_LOG(LogTemp, Log, TEXT("❄️ Jump disabled for %s"), *Target->GetName());
    }
    
    if (bDisableAbilities)
    {
        UE_LOG(LogTemp, Log, TEXT("❄️ Abilities disabled for %s"), *Target->GetName());
    }
    
    UE_LOG(LogTemp, Log, TEXT("❄️ Applying freeze effect: complete immobilization for %f seconds"), 
           FreezeDuration);
}

void ACYFreezeTrap::ShowFreezeVisualEffect()
{
    // 프리즈 트랩 활성화 시 시각적 효과
    if (ItemMesh)
    {
        // 활성화 시 약간 높이 증가 (얼음 생성 효과)
        ItemMesh->SetWorldScale3D(FVector(0.9f, 0.9f, 0.35f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("❄️ Freeze trap visual effects activated"));
}

void ACYFreezeTrap::CreateIceEffect()
{
    // ✅ TrapData에서 이펙트 재생
    if (TrapData.TriggerEffect)
    {
        FVector EffectLocation = GetActorLocation();
        EffectLocation.Z += 50.0f;
        
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrapData.TriggerEffect, EffectLocation);
        UE_LOG(LogTemp, Log, TEXT("❄️ Spawned ice effect from TrapData"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❄️ FreezeTrap: No trigger effect set in TrapData"));
    }
}

void ACYFreezeTrap::PlayFreezeTriggerEffect()
{
    // 트리거 시 전용 효과 (사운드 + 이펙트)
    PlayTrapSound();
    CreateIceEffect();
    
    // 추가적인 프리즈 전용 효과
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, 
            TEXT("💎 FREEZE ACTIVATED! 💎"));
    }
}