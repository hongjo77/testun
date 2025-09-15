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

ACYTrapBase::ACYTrapBase()
{
    ItemName = FText::FromString("Base Trap");
    ItemDescription = FText::FromString("A base trap class");
    ItemTag = FGameplayTag::RequestGameplayTag("Item.Trap");

    MaxStackCount = 5;
    ItemCount = 1;
    TrapType = ETrapType::Slow; // 기본값

    // 트랩은 픽업 불가로 설정
    if (InteractionSphere)
    {
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
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

void ACYTrapBase::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // 트랩 스폰 이벤트
        OnTrapSpawned();
        
        // 타이머 설정
        SetupTrapTimers();
        
        // 시각적 설정
        SetupTrapVisuals();
    }

    UE_LOG(LogTemp, Log, TEXT("🎯 Trap spawned: %s (Type: %d)"), 
           *ItemName.ToString(), static_cast<int32>(TrapType));
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

void ACYTrapBase::SetupTrapTimers()
{
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
    if (!HasAuthority()) return;

    bIsArmed = true;

    // 트리거 영역 설정
    if (!InteractionSphere)
    {
        InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
        InteractionSphere->SetupAttachment(RootComponent);
    }
    
    InteractionSphere->SetSphereRadius(TrapData.TriggerRadius);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // 기존 바인딩 제거 후 트리거 바인딩
    InteractionSphere->OnComponentBeginOverlap.Clear();
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnTriggerSphereOverlap);

    // 트랩 활성화 이벤트
    OnTrapArmed();

    UE_LOG(LogTemp, Log, TEXT("✅ Trap armed and ready: %s"), *ItemName.ToString());
}

void ACYTrapBase::OnTriggerSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsArmed || !HasAuthority() || OtherActor == GetOwner()) return;

    ACYPlayerCharacter* Target = Cast<ACYPlayerCharacter>(OtherActor);
    if (!Target) return;

    // 트랩 트리거 이벤트
    OnTrapTriggered(Target);
    
    // 효과 적용
    ApplyTrapEffects(Target);
    
    // 트랩 파괴
    Destroy();
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