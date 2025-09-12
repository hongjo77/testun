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

    // 🔥 확실하게 기본 효과 추가
    ItemEffects.Empty();
    ItemEffects.Add(UGE_ImmobilizeTrap::StaticClass());
    
    UE_LOG(LogTemp, Warning, TEXT("🏗️ TrapBase created with %d effects"), ItemEffects.Num());
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
    UE_LOG(LogTemp, Warning, TEXT("🚨 TRAP TRIGGERED by: %s"), OtherActor ? *OtherActor->GetName() : TEXT("NULL"));
    
    if (!bIsArmed || !HasAuthority()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Trap not armed or no authority"));
        return;
    }
    
    if (OtherActor == GetOwner()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Owner triggered own trap, ignoring"));
        return;
    }

    ACYPlayerCharacter* Target = Cast<ACYPlayerCharacter>(OtherActor);
    if (!Target) 
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Not a player character"));
        return;
    }

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) 
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ No ASC found on target"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ Target found: %s"), *Target->GetName());
    UE_LOG(LogTemp, Warning, TEXT("📊 ItemEffects count: %d"), ItemEffects.Num());

    // 🔥 ItemEffects가 비어있으면 기본 효과 사용
    if (ItemEffects.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No ItemEffects! Using default ImmobilizeTrap effect"));
        
        // 기본 효과 직접 적용
        FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
        EffectContext.AddSourceObject(this);
        
        FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(UGE_ImmobilizeTrap::StaticClass(), 1, EffectContext);
        if (EffectSpec.IsValid())
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
            UE_LOG(LogTemp, Warning, TEXT("✅ Default ImmobilizeTrap effect applied"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Failed to create default effect spec"));
        }
    }
    else
    {
        // 설정된 효과들 적용
        for (TSubclassOf<UGameplayEffect> EffectClass : ItemEffects)
        {
            if (EffectClass)
            {
                UE_LOG(LogTemp, Warning, TEXT("🎯 Applying effect: %s"), *EffectClass->GetName());
                
                FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
                EffectContext.AddSourceObject(this);
                
                FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
                if (EffectSpec.IsValid())
                {
                    FActiveGameplayEffectHandle ActiveHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
                    if (ActiveHandle.IsValid())
                    {
                        UE_LOG(LogTemp, Warning, TEXT("✅ Effect applied successfully!"));
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("❌ Failed to apply effect"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("❌ Invalid effect spec"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ NULL effect class"));
            }
        }
    }

    // 트랩 제거
    UE_LOG(LogTemp, Warning, TEXT("🗑️ Destroying trap"));
    Destroy();
}