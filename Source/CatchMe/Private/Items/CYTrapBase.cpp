#include "Items/CYTrapBase.h"
#include "Player/CYPlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GAS/CYAttributeSet.h"
#include "GAS/CYGameplayEffects.h"
#include "CYGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Net/UnrealNetwork.h"

ACYTrapBase::ACYTrapBase()
{
    ItemName = FText::FromString("Base Trap");
    ItemDescription = FText::FromString("A base trap class");
    ItemTag = FGameplayTag::RequestGameplayTag("Item.Trap");

    MaxStackCount = 5;
    ItemCount = 1;
    TrapType = ETrapType::Slow; // 기본값
    TrapState = ETrapState::MapPlaced; // ✅ 기본적으로 맵 배치 상태

    // 기본 메시 설정
    if (ItemMesh)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> TrapMeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));
        if (TrapMeshAsset.Succeeded())
        {
            ItemMesh->SetStaticMesh(TrapMeshAsset.Object);
            ItemMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.1f));
        }
        ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ItemMesh->SetVisibility(true);
    }

    // 기본 트랩 데이터 초기화
    TrapData.TrapType = TrapType;
    TrapData.TrapName = ItemName;
    TrapData.TrapDescription = ItemDescription;
    TrapData.TriggerRadius = TriggerRadius;
    TrapData.ArmingDelay = ArmingDelay;
    TrapData.TrapLifetime = TrapLifetime;
}

void ACYTrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACYTrapBase, TrapState);
}

void ACYTrapBase::BeginPlay()
{
    Super::BeginPlay();

    // ✅ 서버와 클라이언트 모두에서 상태별 트랩 설정
    SetupTrapForCurrentState();
    
    if (HasAuthority())
    {
        // 트랩 스폰 이벤트
        OnTrapSpawned();
        
        // 시각적 설정
        SetupTrapVisuals();
    }

    UE_LOG(LogTemp, Warning, TEXT("🎯 Trap BeginPlay: %s (State: %s, Authority: %s)"), 
           *ItemName.ToString(), 
           TrapState == ETrapState::MapPlaced ? TEXT("MapPlaced") : TEXT("PlayerPlaced"),
           HasAuthority() ? TEXT("Server") : TEXT("Client"));
}

void ACYTrapBase::SetupTrapForCurrentState()
{
    if (!InteractionSphere) return;

    if (TrapState == ETrapState::MapPlaced)
    {
        // ✅ 맵 배치 상태: 픽업 가능
        InteractionSphere->SetSphereRadius(150.0f); // 픽업 범위
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        
        // ✅ 기존 바인딩 클리어 후 새로 바인딩
        InteractionSphere->OnComponentBeginOverlap.Clear();
        InteractionSphere->OnComponentEndOverlap.Clear();
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnPickupSphereOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACYTrapBase::OnPickupSphereEndOverlap);
        
        UE_LOG(LogTemp, Warning, TEXT("🎯 Trap set as PICKUPABLE: %s"), *ItemName.ToString());
    }
    else if (TrapState == ETrapState::PlayerPlaced)
    {
        // ✅ 플레이어 배치 상태: 트리거 모드
        if (HasAuthority())
        {
            SetupTrapTimers(); // 서버에서만 타이머 시작
        }
        
        UE_LOG(LogTemp, Warning, TEXT("🎯 Trap set as ACTIVE: %s"), *ItemName.ToString());
    }
}

void ACYTrapBase::ConvertToPlayerPlacedTrap(AActor* PlacingPlayer)
{
    if (!HasAuthority()) return;

    // ✅ 상태 변경
    TrapState = ETrapState::PlayerPlaced;
    
    // ✅ 소유자 설정 (설치한 플레이어)
    SetOwner(PlacingPlayer);
    
    // ✅ 픽업 불가능하게 설정
    bIsPickedUp = true;
    
    // ✅ 상태에 맞게 재설정
    SetupTrapForCurrentState();
    
    UE_LOG(LogTemp, Warning, TEXT("🎯 Trap converted to PlayerPlaced by %s"), 
           PlacingPlayer ? *PlacingPlayer->GetName() : TEXT("Unknown"));
}

void ACYTrapBase::SetupTrapTimers()
{
    // ✅ 플레이어가 설치한 트랩만 타이머 설정
    if (TrapState != ETrapState::PlayerPlaced) return;

    // 트랩 활성화 타이머
    GetWorld()->GetTimerManager().SetTimer(ArmingTimer, this, &ACYTrapBase::ArmTrap, 
                                          TrapData.ArmingDelay, false);
    
    // 트랩 수명 타이머
    GetWorld()->GetTimerManager().SetTimer(LifetimeTimer, [this]()
    {
        Destroy();
    }, TrapData.TrapLifetime, false);
}

void ACYTrapBase::ArmTrap()
{
    if (!HasAuthority() || TrapState != ETrapState::PlayerPlaced) return;

    bIsArmed = true;

    // ✅ 트리거 영역으로 재설정
    if (InteractionSphere)
    {
        InteractionSphere->SetSphereRadius(TrapData.TriggerRadius);
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        
        // 기존 바인딩 제거 후 트리거 바인딩
        InteractionSphere->OnComponentBeginOverlap.Clear();
        InteractionSphere->OnComponentEndOverlap.Clear();
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnTriggerSphereOverlap);
    }

    // 트랩 활성화 이벤트
    OnTrapArmed();

    UE_LOG(LogTemp, Log, TEXT("✅ Trap armed and ready: %s"), *ItemName.ToString());
}

void ACYTrapBase::OnTriggerSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult)
{
    // ✅ 오직 플레이어가 설치한 트랩만 트리거됨
    if (TrapState != ETrapState::PlayerPlaced || !bIsArmed || !HasAuthority()) return;

    // ✅ 설치한 사람은 자신의 트랩에 걸리지 않음
    if (OtherActor == GetOwner())
    {
        UE_LOG(LogTemp, Log, TEXT("🚫 Trap owner stepped on own trap - ignoring"));
        return;
    }

    ACYPlayerCharacter* Target = Cast<ACYPlayerCharacter>(OtherActor);
    if (!Target) return;

    UE_LOG(LogTemp, Warning, TEXT("💥 TRAP TRIGGERED! %s stepped on %s's trap"), 
           *Target->GetName(), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));

    // 트랩 트리거 이벤트
    OnTrapTriggered(Target);
    
    // 효과 적용
    ApplyTrapEffects(Target);
    
    // 트랩 파괴
    Destroy();
}

// ✅ 픽업용 래퍼 함수들 (부모 클래스의 protected 함수 호출)
void ACYTrapBase::OnPickupSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // 부모 클래스의 픽업 함수 호출
    OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void ACYTrapBase::OnPickupSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // 부모 클래스의 픽업 함수 호출
    OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void ACYTrapBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 트랩 파괴 이벤트
    OnTrapDestroyed();
    
    Super::EndPlay(EndPlayReason);
}

void ACYTrapBase::OnTrapSpawned_Implementation()
{
    // 기본 구현 - 하위 클래스에서 오버라이드 가능
    UE_LOG(LogTemp, Log, TEXT("🔧 Base trap spawned"));
}

void ACYTrapBase::OnTrapArmed_Implementation()
{
    // 기본 구현 - 하위 클래스에서 오버라이드 가능
    UE_LOG(LogTemp, Warning, TEXT("⚡ Base trap armed"));
    
    // 사운드 재생
    PlayTrapSound();
}

void ACYTrapBase::OnTrapTriggered_Implementation(ACYPlayerCharacter* Target)
{
    // 기본 구현 - 하위 클래스에서 오버라이드 가능
    if (Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("💥 Base trap triggered on %s"), *Target->GetName());
    }
}

void ACYTrapBase::OnTrapDestroyed_Implementation()
{
    // 기본 구현 - 하위 클래스에서 오버라이드 가능
    UE_LOG(LogTemp, Log, TEXT("🗑️ Base trap destroyed"));
}

void ACYTrapBase::SetupTrapVisuals_Implementation()
{
    // 기본 구현 - 하위 클래스에서 오버라이드
    if (ItemMesh && TrapData.TrapMesh)
    {
        ItemMesh->SetStaticMesh(TrapData.TrapMesh);
    }
    
    // 색상 설정
    if (ItemMesh)
    {
        // Create dynamic material instance and set color
        UMaterialInterface* Material = ItemMesh->GetMaterial(0);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TrapData.TrapColor);
                ItemMesh->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void ACYTrapBase::PlayTrapSound_Implementation()
{
    if (TrapData.TriggerSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), TrapData.TriggerSound, GetActorLocation());
    }
}

void ACYTrapBase::ApplyCustomEffects_Implementation(ACYPlayerCharacter* Target)
{
    // 기본 구현 - 하위 클래스에서 오버라이드
    UE_LOG(LogTemp, Log, TEXT("🎯 Applying base custom effects"));
}

void ACYTrapBase::ApplyTrapEffects(ACYPlayerCharacter* Target)
{
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    UE_LOG(LogTemp, Warning, TEXT("🎯 Applying trap effects to %s"), *Target->GetName());

    // 기본 게임플레이 효과들 적용
    for (TSubclassOf<UGameplayEffect> EffectClass : TrapData.GameplayEffects)
    {
        if (EffectClass)
        {
            ApplySingleEffect(TargetASC, EffectClass);
        }
    }

    // 레거시 ItemEffects도 적용 (호환성)
    for (TSubclassOf<UGameplayEffect> EffectClass : ItemEffects)
    {
        if (EffectClass)
        {
            ApplySingleEffect(TargetASC, EffectClass);
        }
    }

    // 하위 클래스별 커스텀 효과
    ApplyCustomEffects(Target);

    UE_LOG(LogTemp, Log, TEXT("✅ Applied %d trap effects"), 
           TrapData.GameplayEffects.Num() + ItemEffects.Num());
}

void ACYTrapBase::ApplySingleEffect(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass)
{
    if (!TargetASC || !EffectClass) return;

    FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);
    
    FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
    if (EffectSpec.IsValid())
    {
        TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
        UE_LOG(LogTemp, Log, TEXT("Applied effect: %s"), *EffectClass->GetName());
    }
}