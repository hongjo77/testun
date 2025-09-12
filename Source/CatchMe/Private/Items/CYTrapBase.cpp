#include "Items/CYTrapBase.h"
#include "Player/CYPlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "GAS/CYAttributeSet.h"
#include "GAS/CYGameplayEffects.h"
#include "CYGameplayTags.h"

ACYTrapBase::ACYTrapBase()
{
    ItemName = FText::FromString("Trap");
    ItemDescription = FText::FromString("A placeable trap");
    ItemTag = FGameplayTag::RequestGameplayTag("Item.Trap");

    MaxStackCount = 5;
    ItemCount = 1;

    // 트랩은 픽업 불가
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
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

    // ✅ 기본값 설정 제거 - GA_PlaceTrap에서 처리
}

void ACYTrapBase::BeginPlay()
{
    Super::BeginPlay();

    // ✅ ItemEffects 검증만 수행 (설정은 GA_PlaceTrap에서 완료)
    UE_LOG(LogTemp, Log, TEXT("Trap armed with %d effects"), ItemEffects.Num());

    if (HasAuthority())
    {
        SetupTrapTimers();
    }
}

void ACYTrapBase::SetupTrapTimers()
{
    // 트랩 활성화 타이머
    GetWorld()->GetTimerManager().SetTimer(ArmingTimer, this, &ACYTrapBase::ArmTrap, ArmingDelay, false);
    
    // 트랩 수명 타이머
    GetWorld()->GetTimerManager().SetTimer(LifetimeTimer, [this]()
    {
        Destroy();
    }, TrapLifetime, false);
}

void ACYTrapBase::ArmTrap()
{
    if (!HasAuthority()) return;

    bIsArmed = true;

    // ✅ 이 시점에서 효과 개수 확인 및 로그 출력
    UE_LOG(LogTemp, Warning, TEXT("🎯 Trap armed with %d effects"), ItemEffects.Num());
    for (int32 i = 0; i < ItemEffects.Num(); i++)
    {
        if (ItemEffects[i])
        {
            UE_LOG(LogTemp, Warning, TEXT("  Effect[%d]: %s"), i, *ItemEffects[i]->GetName());
        }
    }

    // 트리거 영역 설정
    if (!InteractionSphere)
    {
        InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
        InteractionSphere->SetupAttachment(RootComponent);
    }
    
    InteractionSphere->SetSphereRadius(TriggerRadius);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // 기존 바인딩 제거 후 트리거 바인딩
    InteractionSphere->OnComponentBeginOverlap.Clear();
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnTrapTriggered);

    UE_LOG(LogTemp, Log, TEXT("✅ Trap armed and ready"));
}

void ACYTrapBase::OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsArmed || !HasAuthority() || OtherActor == GetOwner()) return;

    ACYPlayerCharacter* Target = Cast<ACYPlayerCharacter>(OtherActor);
    if (!Target) return;

    ApplyTrapEffectsToTarget(Target);
    Destroy();
}

void ACYTrapBase::ApplyTrapEffectsToTarget(ACYPlayerCharacter* Target)
{
    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    UE_LOG(LogTemp, Warning, TEXT("Trap triggered on %s with %d effects"), 
           *Target->GetName(), ItemEffects.Num());

    // 모든 효과 적용
    for (TSubclassOf<UGameplayEffect> EffectClass : ItemEffects)
    {
        if (EffectClass)
        {
            ApplySingleEffect(TargetASC, EffectClass);
        }
    }
}

void ACYTrapBase::ApplySingleEffect(UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass)
{
    FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);
    
    FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
    if (EffectSpec.IsValid())
    {
        TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
        UE_LOG(LogTemp, Log, TEXT("Applied effect: %s"), *EffectClass->GetName());
    }
}