#include "Items/CYItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Player/CYPlayerCharacter.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

ACYItemBase::ACYItemBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    // Root Component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // Mesh Component
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
    ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    // Interaction Sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ACYItemBase::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYItemBase::OnSphereOverlap);
    }
}

void ACYItemBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(OtherActor))
    {
        // UI 힌트 표시를 위한 이벤트 (블루프린트에서 처리)
        UE_LOG(LogTemp, Warning, TEXT("Player near item: %s"), *ItemName.ToString());
    }
}

void ACYItemBase::OnPickup(ACYPlayerCharacter* Character)
{
    if (!Character) return;

    UCYAbilitySystemComponent* ASC = Cast<UCYAbilitySystemComponent>(Character->GetAbilitySystemComponent());
    if (!ASC) return;

    // Grant item ability
    if (ItemAbility)
    {
        FGameplayAbilitySpecHandle Handle = ASC->GiveItemAbility(ItemAbility, 1);
        UE_LOG(LogTemp, Warning, TEXT("Granted ability for item: %s"), *ItemName.ToString());
    }

    // Apply item effects
    for (TSubclassOf<UGameplayEffect> EffectClass : ItemEffects)
    {
        if (EffectClass)
        {
            FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
            ContextHandle.AddSourceObject(this);
            
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1, ContextHandle);
            if (SpecHandle.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }
}

void ACYItemBase::ServerPickup_Implementation(ACYPlayerCharacter* Character)
{
    if (!HasAuthority() || !Character) return;

    OnPickup(Character);

    // 아이템 숨기기
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    // 일정 시간 후 제거
    SetLifeSpan(2.0f);
}