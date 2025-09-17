// CYItemBase.cpp - 어빌리티 중복 등록 방지
#include "Items/CYItemBase.h"
#include "CYGameplayTags.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Player/CYPlayerCharacter.h"
#include "GAS/CYAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Items/CYWeaponBase.h"

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
    
    // ✅ 중요: ObjectType을 WorldDynamic으로 설정
    InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);

    // 기본 아이템 태그
    const FCYGameplayTags& GameplayTags = FCYGameplayTags::Get();
    ItemTag = GameplayTags.Item_Base;
    
    bIsPickedUp = false;
    ItemCount = 1;
    MaxStackCount = 10;
}

void ACYItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ACYItemBase, bIsPickedUp);
    DOREPLIFETIME(ACYItemBase, ItemCount); 
}

void ACYItemBase::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYItemBase::OnSphereOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACYItemBase::OnSphereEndOverlap);
    }
}

void ACYItemBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsPickedUp) return;

    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(OtherActor))
    {
        // UI 힌트 표시를 위한 이벤트
        UE_LOG(LogTemp, Warning, TEXT("Player near item: %s"), *ItemName.ToString());
        
        // 클라이언트에서 UI 업데이트
        if (Character->IsLocallyControlled())
        {
            OnPlayerNearItem.Broadcast(this, true);
        }
    }
}

void ACYItemBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ACYPlayerCharacter* Character = Cast<ACYPlayerCharacter>(OtherActor))
    {
        // UI 힌트 숨기기
        if (Character->IsLocallyControlled())
        {
            OnPlayerNearItem.Broadcast(this, false);
        }
    }
}

void ACYItemBase::OnPickup(ACYPlayerCharacter* Character)
{
    if (!Character || bIsPickedUp) return;

    UCYAbilitySystemComponent* ASC = Cast<UCYAbilitySystemComponent>(Character->GetAbilitySystemComponent());
    if (!ASC) return;

    // ✅ 어빌리티 중복 등록 방지 - 트랩 아이템은 어빌리티를 등록하지 않음
    FGameplayTag TrapTag = FGameplayTag::RequestGameplayTag("Item.Trap");
    if (!ItemTag.MatchesTag(TrapTag) && ItemAbility && !ItemAbilityHandle.IsValid())
    {
        ItemAbilityHandle = ASC->GiveItemAbility(ItemAbility, 1);
        UE_LOG(LogTemp, Warning, TEXT("Granted ability for item: %s"), *ItemName.ToString());
    }
    else if (ItemTag.MatchesTag(TrapTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("Trap item picked up: %s (No ability granted - using central trap ability)"), *ItemName.ToString());
    }
    else if (ItemAbilityHandle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Ability already granted for item: %s"), *ItemName.ToString());
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

    // 아이템이 픽업되었음을 표시
    bIsPickedUp = true;
    
    // 픽업 이벤트 발생
    OnItemPickedUp.Broadcast(this, Character);
}

void ACYItemBase::OnDrop(ACYPlayerCharacter* Character)
{
    if (!Character) return;

    UCYAbilitySystemComponent* ASC = Cast<UCYAbilitySystemComponent>(Character->GetAbilitySystemComponent());
    if (!ASC) return;

    // Remove item ability
    if (ItemAbilityHandle.IsValid())
    {
        ASC->RemoveItemAbility(ItemAbilityHandle);
        // FGameplayAbilitySpecHandle을 무효화하는 올바른 방법
        ItemAbilityHandle = FGameplayAbilitySpecHandle();  // 기본 생성자로 초기화 = 무효한 핸들
    }

    // 아이템 드랍 로직
    bIsPickedUp = false;
    
    // 충돌 다시 활성화
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    SetActorHiddenInGame(false);
    
    // 드랍 이벤트
    OnItemDropped.Broadcast(this, Character);
}

void ACYItemBase::ServerPickup_Implementation(ACYPlayerCharacter* Character)
{
    if (!HasAuthority() || !Character || bIsPickedUp) return;

    OnPickup(Character);

    // 아이템 숨기기 (무기가 아닌 경우에만)
    if (!IsA<ACYWeaponBase>())
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        
        // 일정 시간 후 제거 또는 리스폰
        SetLifeSpan(2.0f);
    }
}

void ACYItemBase::OnRep_bIsPickedUp()
{
    if (bIsPickedUp)
    {
        // 클라이언트에서 아이템 숨기기
        if (!IsA<ACYWeaponBase>())
        {
            SetActorHiddenInGame(true);
            SetActorEnableCollision(false);
        }
    }
    else
    {
        // 아이템이 드랍되었을 때
        SetActorHiddenInGame(false);
        SetActorEnableCollision(true);
    }
}

bool ACYItemBase::CanStackWith(ACYItemBase* OtherItem) const
{
    if (!OtherItem) return false;
    
    // ✅ 더 관대한 스택 조건
    // 1. 같은 클래스여야 함
    // 2. 스택 가능해야 함 (MaxStackCount > 1)
    // 3. 다른 아이템이 최대 스택이 아니어야 함
    bool bSameClass = (GetClass() == OtherItem->GetClass());
    bool bStackable = (MaxStackCount > 1);
    bool bHasSpace = (OtherItem->ItemCount < OtherItem->MaxStackCount);
    
    UE_LOG(LogTemp, Warning, TEXT("CanStackWith: SameClass=%s, Stackable=%s, HasSpace=%s"), 
           bSameClass ? TEXT("true") : TEXT("false"),
           bStackable ? TEXT("true") : TEXT("false"),
           bHasSpace ? TEXT("true") : TEXT("false"));
    
    return bSameClass && bStackable && bHasSpace;
}