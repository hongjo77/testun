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

    // ✅ 생성자에서는 기본값 설정 안함 (GA_PlaceTrap에서 설정하도록)
}

void ACYTrapBase::BeginPlay()
{
    Super::BeginPlay();

    // ✅ BeginPlay에서만 ItemEffects가 비어있으면 기본값 설정
    if (ItemEffects.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No custom effects, using default ImmobilizeTrap"));
        ItemEffects.Add(UGE_ImmobilizeTrap::StaticClass());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Trap using %d custom effects"), ItemEffects.Num());
    }

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

    if (!InteractionSphere)
    {
        InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
        InteractionSphere->SetupAttachment(RootComponent);
    }
    
    InteractionSphere->SetSphereRadius(TriggerRadius);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    InteractionSphere->OnComponentBeginOverlap.Clear();
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnTrapTriggered);

    UE_LOG(LogTemp, Warning, TEXT("Trap armed"));
}

void ACYTrapBase::OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsArmed || !HasAuthority() || OtherActor == GetOwner()) return;

    ACYPlayerCharacter* Target = Cast<ACYPlayerCharacter>(OtherActor);
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    UE_LOG(LogTemp, Warning, TEXT("Trap triggered on %s with %d effects"), 
           *Target->GetName(), ItemEffects.Num());

    // 효과들 적용
    for (TSubclassOf<UGameplayEffect> EffectClass : ItemEffects)
    {
        if (EffectClass)
        {
            FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
            EffectContext.AddSourceObject(this);
            
            FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
            if (EffectSpec.IsValid())
            {
                TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
                UE_LOG(LogTemp, Warning, TEXT("Applied effect: %s"), *EffectClass->GetName());
            }
        }
    }

    Destroy();
}