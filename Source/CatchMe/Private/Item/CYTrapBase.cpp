// Item/CYTrapBase.cpp
#include "Item/CYTrapBase.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

ACYTrapBase::ACYTrapBase()
{
    bCanPickup = false;

    TriggerCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerCollision"));
    TriggerCollision->SetupAttachment(RootComponent);
    TriggerCollision->SetSphereRadius(100.0f);
    TriggerCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    TriggerCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    TrapLifetime = 120.0f;
    bSingleUse = true;
    ArmingDelay = 2.0f;
    bTriggered = false;
    bArmed = false;
}

void ACYTrapBase::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        GetWorld()->GetTimerManager().SetTimer(ArmingTimer, this, &ACYTrapBase::ArmTrap, ArmingDelay, false);
        GetWorld()->GetTimerManager().SetTimer(LifetimeTimer, this, &ACYTrapBase::RemoveTrap, TrapLifetime, false);
    }
}

void ACYTrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ACYTrapBase, bTriggered);
    DOREPLIFETIME(ACYTrapBase, bArmed);
}

void ACYTrapBase::ArmTrap()
{
    if (!HasAuthority()) return;

    bArmed = true;
    
    if (TriggerCollision)
    {
        TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &ACYTrapBase::OnTrapTriggered);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Trap %s is now armed"), *GetName());
}

void ACYTrapBase::OnTrapTriggered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                 bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || !bArmed || bTriggered) return;
    if (!CanTrigger(OtherActor)) return;

    bTriggered = true;
    ApplyTrapEffect(OtherActor);
    MulticastPlayTrapEffect();

    UE_LOG(LogTemp, Warning, TEXT("Trap %s triggered by %s"), *GetName(), *OtherActor->GetName());

    if (bSingleUse)
    {
        GetWorld()->GetTimerManager().SetTimer(RemovalTimer, this, &ACYTrapBase::RemoveTrap, 1.0f, false);
    }
}

void ACYTrapBase::ApplyTrapEffect(AActor* Target)
{
    // 기본 구현 비어있음 - 자식 클래스에서 오버라이드
}

bool ACYTrapBase::CanTrigger(AActor* Target)
{
    ACharacter* Character = Cast<ACharacter>(Target);
    if (!Character) return false;
    if (Target == GetOwner()) return false;
    return true;
}

void ACYTrapBase::RemoveTrap()
{
    if (HasAuthority())
    {
        Destroy();
    }
}

void ACYTrapBase::MulticastPlayTrapEffect_Implementation()
{
    // 파티클, 사운드 재생
}