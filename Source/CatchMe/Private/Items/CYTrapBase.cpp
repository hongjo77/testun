// CYTrapBase.cpp - 클라이언트 메시 동기화 개선
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
    TrapType = ETrapType::Slow;
    TrapState = ETrapState::MapPlaced;

    // ✅ 네트워킹 설정 강화
    bReplicates = true;
    SetReplicateMovement(true);
    bAlwaysRelevant = true;

    // ✅ 기본 메시 설정 개선
    if (ItemMesh)
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> TrapMeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));
        if (TrapMeshAsset.Succeeded())
        {
            ItemMesh->SetStaticMesh(TrapMeshAsset.Object);
            ItemMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.1f));
        }
        
        // ✅ 기본 메시 설정 강화
        ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ItemMesh->SetVisibility(true);
        ItemMesh->SetHiddenInGame(false); // 명시적으로 표시
        ItemMesh->SetCastShadow(true); // 그림자 활성화
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
    DOREPLIFETIME(ACYTrapBase, bIsArmed);
    // ✅ TrapType도 리플리케이트하여 클라이언트에서 올바른 타입 인식
    DOREPLIFETIME(ACYTrapBase, TrapType);
    DOREPLIFETIME(ACYTrapBase, TrapData);
}

void ACYTrapBase::BeginPlay()
{
    Super::BeginPlay();

    // ✅ 모든 클라이언트에서 상태별 트랩 설정
    SetupTrapForCurrentState();
    
    // ✅ 서버와 클라이언트 모두에서 비주얼 설정
    SetupTrapVisuals();
    
    // ✅ 서버에서만 로직 처리
    if (HasAuthority())
    {
        OnTrapSpawned();
    }

    UE_LOG(LogTemp, Warning, TEXT("🎯 Trap BeginPlay: %s (State: %s, Authority: %s, CollisionEnabled: %s)"), 
           *ItemName.ToString(), 
           TrapState == ETrapState::MapPlaced ? TEXT("MapPlaced") : TEXT("PlayerPlaced"),
           HasAuthority() ? TEXT("Server") : TEXT("Client"),
           InteractionSphere ? (InteractionSphere->GetCollisionEnabled() != ECollisionEnabled::NoCollision ? TEXT("Enabled") : TEXT("Disabled")) : TEXT("NULL"));
}

void ACYTrapBase::SetupTrapForCurrentState()
{
    if (!InteractionSphere) return;

    if (TrapState == ETrapState::MapPlaced)
    {
        // ✅ 맵 배치 상태: 픽업 가능하도록 설정
        InteractionSphere->SetSphereRadius(150.0f);
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
        
        // ✅ 서버에서만 픽업 바인딩
        if (HasAuthority())
        {
            InteractionSphere->OnComponentBeginOverlap.Clear();
            InteractionSphere->OnComponentEndOverlap.Clear();
            InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnPickupSphereOverlap);
            InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACYTrapBase::OnPickupSphereEndOverlap);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("🎯 Trap set as PICKUPABLE: %s (Radius: %f)"), 
               *ItemName.ToString(), InteractionSphere->GetScaledSphereRadius());
    }
    else if (TrapState == ETrapState::PlayerPlaced)
    {
        // ✅ 플레이어 배치 상태: 트리거 모드
        if (HasAuthority())
        {
            SetupTrapTimers();
        }
        
        UE_LOG(LogTemp, Warning, TEXT("🎯 Trap set as ACTIVE: %s"), *ItemName.ToString());
    }
}

void ACYTrapBase::ConvertToPlayerPlacedTrap(AActor* PlacingPlayer)
{
    if (!HasAuthority()) return;

    TrapState = ETrapState::PlayerPlaced;
    SetOwner(PlacingPlayer);
    bIsPickedUp = true;
    
    SetupTrapForCurrentState();
    
    // ✅ 클라이언트들에게 즉시 시각적 업데이트 알림
    MulticastUpdateTrapVisuals();
    
    // ✅ 추가: 네트워크 업데이트 강제 실행
    ForceNetUpdate();
    
    UE_LOG(LogTemp, Warning, TEXT("🎯 Trap converted to PlayerPlaced by %s"), 
           PlacingPlayer ? *PlacingPlayer->GetName() : TEXT("Unknown"));
}

void ACYTrapBase::SetupTrapTimers()
{
    if (TrapState != ETrapState::PlayerPlaced) return;

    GetWorld()->GetTimerManager().SetTimer(ArmingTimer, this, &ACYTrapBase::ArmTrap, 
                                          TrapData.ArmingDelay, false);
    
    GetWorld()->GetTimerManager().SetTimer(LifetimeTimer, [this]()
    {
        Destroy();
    }, TrapData.TrapLifetime, false);
}

void ACYTrapBase::ArmTrap()
{
    if (!HasAuthority() || TrapState != ETrapState::PlayerPlaced) return;

    bIsArmed = true;

    if (InteractionSphere)
    {
        InteractionSphere->SetSphereRadius(TrapData.TriggerRadius);
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        
        InteractionSphere->OnComponentBeginOverlap.Clear();
        InteractionSphere->OnComponentEndOverlap.Clear();
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnTriggerSphereOverlap);
    }

    ForceNetUpdate();
    OnTrapArmed();

    UE_LOG(LogTemp, Log, TEXT("✅ Trap armed and ready: %s"), *ItemName.ToString());
}

void ACYTrapBase::OnTriggerSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult)
{
    if (TrapState != ETrapState::PlayerPlaced || !bIsArmed || !HasAuthority()) return;

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

    OnTrapTriggered(Target);
    ApplyTrapEffects(Target);
    MulticastOnTrapTriggered(Target);
    
    Destroy();
}

// ✅ 새로운 멀티캐스트 함수 - 시각적 업데이트
void ACYTrapBase::MulticastUpdateTrapVisuals_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("🎨 MulticastUpdateTrapVisuals called on %s"), 
           HasAuthority() ? TEXT("Server") : TEXT("Client"));
    
    // ✅ 클라이언트에서도 강제로 시각적 설정
    SetupTrapVisuals();
    
    // ✅ 메시 가시성 강제 설정
    if (ItemMesh)
    {
        ItemMesh->SetVisibility(true);
        ItemMesh->SetHiddenInGame(false);
        ItemMesh->MarkRenderStateDirty();
        
        UE_LOG(LogTemp, Warning, TEXT("🎨 Client mesh visibility forced: %s"), 
               ItemMesh->IsVisible() ? TEXT("true") : TEXT("false"));
    }
}

void ACYTrapBase::MulticastOnTrapTriggered_Implementation(ACYPlayerCharacter* Target)
{
    if (!HasAuthority()) // 클라이언트에서만 실행
    {
        OnTrapTriggered(Target);
        SetupTrapVisuals(); // 트리거 시각 효과
    }
}

void ACYTrapBase::OnPickupSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void ACYTrapBase::OnPickupSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void ACYTrapBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    OnTrapDestroyed();
    Super::EndPlay(EndPlayReason);
}

void ACYTrapBase::OnTrapSpawned_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("🔧 Base trap spawned"));
}

void ACYTrapBase::OnTrapArmed_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("⚡ Base trap armed"));
    PlayTrapSound();
}

void ACYTrapBase::OnTrapTriggered_Implementation(ACYPlayerCharacter* Target)
{
    if (Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("💥 Base trap triggered on %s"), *Target->GetName());
    }
}

void ACYTrapBase::OnTrapDestroyed_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("🗑️ Base trap destroyed"));
}

void ACYTrapBase::SetupTrapVisuals_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("🎨 SetupTrapVisuals called on %s"), 
           HasAuthority() ? TEXT("Server") : TEXT("Client"));

    if (ItemMesh)
    {
        // ✅ 기본 메시가 없으면 설정
        if (!ItemMesh->GetStaticMesh())
        {
            static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Engine/BasicShapes/Cylinder"));
            if (DefaultMesh.Succeeded())
            {
                ItemMesh->SetStaticMesh(DefaultMesh.Object);
                UE_LOG(LogTemp, Warning, TEXT("🎨 Set default cylinder mesh"));
            }
        }
        
        // ✅ TrapData에서 메시가 있으면 사용
        if (TrapData.TrapMesh)
        {
            ItemMesh->SetStaticMesh(TrapData.TrapMesh);
            UE_LOG(LogTemp, Warning, TEXT("🎨 Set TrapData mesh: %s"), *TrapData.TrapMesh->GetName());
        }
        
        // ✅ 기본 스케일 보장
        FVector CurrentScale = ItemMesh->GetComponentScale();
        if (CurrentScale.IsNearlyZero())
        {
            ItemMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.1f));
        }
        
        // ✅ 가시성 강제 보장
        ItemMesh->SetVisibility(true);
        ItemMesh->SetHiddenInGame(false);
        ItemMesh->SetCastShadow(true);
        
        // ✅ 렌더 상태 업데이트 강제
        ItemMesh->MarkRenderStateDirty();
        
        // ✅ 머티리얼 설정 개선 - 중복 생성 방지
        UMaterialInterface* Material = ItemMesh->GetMaterial(0);
        if (Material && !Material->IsA<UMaterialInstanceDynamic>())
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TrapData.TrapColor);
                ItemMesh->SetMaterial(0, DynamicMaterial);
                
                UE_LOG(LogTemp, Warning, TEXT("🎨 Applied dynamic material with color %s"), 
                       *TrapData.TrapColor.ToString());
            }
        }
        else if (!Material)
        {
            // ✅ 머티리얼이 없으면 기본 머티리얼 생성
            UMaterialInterface* DefaultMat = UMaterial::GetDefaultMaterial(MD_Surface);
            if (DefaultMat)
            {
                UMaterialInstanceDynamic* DefaultMaterial = UMaterialInstanceDynamic::Create(DefaultMat, this);
                if (DefaultMaterial)
                {
                    DefaultMaterial->SetVectorParameterValue(TEXT("BaseColor"), TrapData.TrapColor);
                    ItemMesh->SetMaterial(0, DefaultMaterial);
                    
                    UE_LOG(LogTemp, Warning, TEXT("🎨 Created default material with color %s"), 
                           *TrapData.TrapColor.ToString());
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("🎨 Trap visuals setup complete: Mesh=%s, Scale=%s, Visible=%s, Hidden=%s"), 
               ItemMesh->GetStaticMesh() ? *ItemMesh->GetStaticMesh()->GetName() : TEXT("NULL"),
               *ItemMesh->GetComponentScale().ToString(),
               ItemMesh->IsVisible() ? TEXT("true") : TEXT("false"),
               ItemMesh->bHiddenInGame ? TEXT("true") : TEXT("false"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ItemMesh is NULL in SetupTrapVisuals"));
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
    UE_LOG(LogTemp, Log, TEXT("🎯 Applying base custom effects"));
}

void ACYTrapBase::ApplyTrapEffects(ACYPlayerCharacter* Target)
{
    if (!Target) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!TargetASC) return;

    UE_LOG(LogTemp, Warning, TEXT("🎯 Applying trap effects to %s"), *Target->GetName());

    for (TSubclassOf<UGameplayEffect> EffectClass : TrapData.GameplayEffects)
    {
        if (EffectClass)
        {
            ApplySingleEffect(TargetASC, EffectClass);
        }
    }

    for (TSubclassOf<UGameplayEffect> EffectClass : ItemEffects)
    {
        if (EffectClass)
        {
            ApplySingleEffect(TargetASC, EffectClass);
        }
    }

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