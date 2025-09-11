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

    // ✅ 트랩은 최대 5개까지 스택 가능
    MaxStackCount = 5;
    ItemCount = 1;

    // 트랩은 픽업 불가
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
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

    ItemEffects.Add(UGE_ImmobilizeTrap::StaticClass());
}

void ACYTrapBase::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        GetWorld()->GetTimerManager().SetTimer(ArmingTimer, this, &ACYTrapBase::ArmTrap, ArmingDelay, false);
        GetWorld()->GetTimerManager().SetTimer(LifetimeTimer, [this]()
        {
            Destroy();
        }, TrapLifetime, false);
    }
}

void ACYTrapBase::ArmTrap()
{
    bIsArmed = true;

    // 기존 InteractionSphere를 트리거로 재사용하거나 새로 생성
    if (!InteractionSphere)
    {
        InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
        InteractionSphere->SetupAttachment(RootComponent);
    }
    
    // 트리거 설정
    InteractionSphere->SetSphereRadius(TriggerRadius);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // 기존 델리게이트 제거 후 트랩 트리거 바인딩
    InteractionSphere->OnComponentBeginOverlap.Clear();
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnTrapTriggered);

    UE_LOG(LogTemp, Warning, TEXT("Trap armed at location: %s"), *GetActorLocation().ToString());
}

void ACYTrapBase::OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("Trap triggered by: %s"), OtherActor ? *OtherActor->GetName() : TEXT("NULL"));
    
    if (!bIsArmed || !HasAuthority()) return;
    if (OtherActor == GetOwner()) return;

    ACYPlayerCharacter* Target = Cast<ACYPlayerCharacter>(OtherActor);
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    // ItemEffects 적용
    UE_LOG(LogTemp, Warning, TEXT("ItemEffects count: %d"), ItemEffects.Num());
    
    for (TSubclassOf<UGameplayEffect> EffectClass : ItemEffects)
    {
        if (EffectClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("Applying effect: %s"), *EffectClass->GetName());
            
            FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
            EffectContext.AddSourceObject(this);
            
            FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
            if (EffectSpec.IsValid())
            {
                TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
                UE_LOG(LogTemp, Warning, TEXT("Effect applied successfully"));
            }
        }
    }

    if (ItemEffects.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No ItemEffects configured"));
    }

    Destroy();
}