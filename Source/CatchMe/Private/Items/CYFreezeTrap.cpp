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
    
    // ✅ 트랩 배치 어빌리티 설정
    // ItemAbility = UGA_PlaceTrap::StaticClass(); // 블루프린트에서 설정하는 것이 더 안전
    
    // 프리즈 트랩 설정
    TriggerRadius = 100.0f;
    FreezeDuration = 3.0f;
    bDisableJumping = true;
    bDisableAbilities = true;

    // 트랩 데이터 설정
    TrapData.TrapType = ETrapType::Freeze;
    TrapData.TrapName = ItemName;
    TrapData.TrapDescription = ItemDescription;
    TrapData.TriggerRadius = TriggerRadius;
    TrapData.TrapColor = FLinearColor::Green; // 청록색으로 구분

    // 프리즈 효과 설정 (완전 정지)
    TrapData.GameplayEffects.Empty();
    TrapData.GameplayEffects.Add(UGE_ImmobilizeTrap::StaticClass());

    // 시각적 설정
    if (ItemMesh)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> FreezeTrapMesh(TEXT("/Engine/BasicShapes/Cylinder"));
        if (FreezeTrapMesh.Succeeded())
        {
            ItemMesh->SetStaticMesh(FreezeTrapMesh.Object);
            ItemMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.1f));
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
}

void ACYFreezeTrap::SetupTrapVisuals_Implementation()
{
    Super::SetupTrapVisuals_Implementation();
    
    // 프리즈 트랩만의 추가 시각적 설정
    if (ItemMesh)
    {
        // 얼음/크리스탈 머티리얼 적용
        UMaterialInterface* Material = ItemMesh->GetMaterial(0);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Green);
                DynamicMaterial->SetScalarParameterValue(TEXT("Metallic"), 0.9f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.1f);
                DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), 0.3f); // 약간의 발광
                ItemMesh->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void ACYFreezeTrap::PlayTrapSound_Implementation()
{
    Super::PlayTrapSound_Implementation();
    
    // 프리즈 트랩만의 사운드 (얼음 깨지는 소리, 바람 소리 등)
    // TODO: 프리즈 트랩 전용 사운드 추가
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
    // 예: 점프 비활성화, 어빌리티 사용 금지 등
    
    if (bDisableJumping)
    {
        // TODO: 점프 비활성화 효과 적용
        UE_LOG(LogTemp, Log, TEXT("❄️ Jump disabled for %s"), *Target->GetName());
    }
    
    if (bDisableAbilities)
    {
        // TODO: 어빌리티 사용 금지 효과 적용
        UE_LOG(LogTemp, Log, TEXT("❄️ Abilities disabled for %s"), *Target->GetName());
    }
    
    UE_LOG(LogTemp, Log, TEXT("❄️ Applying freeze effect: complete immobilization for %f seconds"), 
           FreezeDuration);
}

void ACYFreezeTrap::ShowFreezeVisualEffect()
{
    // 프리즈 트랩 활성화 시 시각적 효과
    // 예: 얼음 크리스탈, 차가운 안개 등
    
    if (ItemMesh)
    {
        // 활성화 시 약간 높이 증가 (얼음 생성 효과)
        ItemMesh->SetWorldScale3D(FVector(0.6f, 0.6f, 0.15f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("❄️ Freeze trap visual effects activated"));
}

void ACYFreezeTrap::CreateIceEffect()
{
    // 트랩 트리거 시 얼음 효과 생성
    // 예: 얼음 파티클, 서리 효과 등
    
    FVector EffectLocation = GetActorLocation();
    EffectLocation.Z += 50.0f; // 약간 위에 효과 생성
    
    // TODO: 얼음 파티클 시스템 추가
    // if (IceParticleSystem)
    // {
    //     UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), IceParticleSystem, EffectLocation);
    // }
    
    UE_LOG(LogTemp, Log, TEXT("❄️ Ice effect created at location %s"), *EffectLocation.ToString());
}