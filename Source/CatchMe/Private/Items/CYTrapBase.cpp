#include "Items/CYTrapBase.h"
#include "Player/CYPlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"

ACYTrapBase::ACYTrapBase()
{
    ItemName = FText::FromString("Trap");
    ItemDescription = FText::FromString("A placeable trap");
    ItemTag = FGameplayTag::RequestGameplayTag("Item.Trap");

    // 트랩은 픽업 불가
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACYTrapBase::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // 활성화 딜레이
        GetWorld()->GetTimerManager().SetTimer(ArmingTimer, this, &ACYTrapBase::ArmTrap, ArmingDelay, false);

        // 수명 타이머
        GetWorld()->GetTimerManager().SetTimer(LifetimeTimer, [this]()
        {
            Destroy();
        }, TrapLifetime, false);
    }
}

void ACYTrapBase::ArmTrap()
{
    bIsArmed = true;

    // 트리거 콜리전 생성 및 설정
    if (USphereComponent* TriggerSphere = NewObject<USphereComponent>(this))
    {
        TriggerSphere->RegisterComponent();
        TriggerSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
        TriggerSphere->SetSphereRadius(TriggerRadius);
        TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnTrapTriggered);
    }

    UE_LOG(LogTemp, Warning, TEXT("Trap armed!"));
}

void ACYTrapBase::OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsArmed || !HasAuthority()) return;

    // 자기 자신이나 같은 팀은 무시
    if (OtherActor == GetOwner()) return;

    ACYPlayerCharacter* Target = Cast<ACYPlayerCharacter>(OtherActor);
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC || !TrapEffectClass) return;

    // 트랩 효과 적용
    FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);
    
    FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(TrapEffectClass, 1, EffectContext);
    if (EffectSpec.IsValid())
    {
        TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
        UE_LOG(LogTemp, Warning, TEXT("Trap triggered on %s"), *Target->GetName());
    }

    // 트랩 제거
    Destroy();
}