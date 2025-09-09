// Item/CYItemBase.cpp
#include "Item/CYItemBase.h"
#include "Systems/CYInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ACYItemBase::ACYItemBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    // 루트 컴포넌트 설정
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // 메쉬 컴포넌트 생성
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ItemMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // 픽업 콜리전 생성
    PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
    PickupCollision->SetupAttachment(RootComponent);
    PickupCollision->SetSphereRadius(150.0f);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    PickupCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    PickupCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // 기본값 설정
    ItemName = FText::FromString("Default Item");
    ItemDescription = FText::FromString("A basic item");
    bCanPickup = true;
}

void ACYItemBase::BeginPlay()
{
    Super::BeginPlay();

    // 픽업 콜리전 이벤트 바인딩
    if (PickupCollision)
    {
        PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &ACYItemBase::OnPickupOverlap);
    }
}

void ACYItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ACYItemBase, ItemType);
    DOREPLIFETIME(ACYItemBase, bCanPickup);
}

void ACYItemBase::UseItem(AActor* User)
{
    if (!User) return;

    // C++ 로직 실행
    ExecuteItemEffect(User);
    
    // 블루프린트 이벤트 호출
    OnItemUsed(User);
    
    UE_LOG(LogTemp, Warning, TEXT("%s used by %s"), *ItemName.ToString(), *User->GetName());
}

void ACYItemBase::ServerPickupItem_Implementation(AActor* User)
{
    UE_LOG(LogTemp, Warning, TEXT("ServerPickupItem called by %s"), User ? *User->GetName() : TEXT("NULL"));
    
    if (!HasAuthority() || !bCanPickup || !User) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Pickup failed: Authority=%d, CanPickup=%d, User=%s"), 
               HasAuthority(), bCanPickup, User ? TEXT("Valid") : TEXT("NULL"));
        return;
    }

    UCYInventoryComponent* Inventory = User->FindComponentByClass<UCYInventoryComponent>();
    if (!Inventory) 
    {
        UE_LOG(LogTemp, Warning, TEXT("User %s has no inventory component"), *User->GetName());
        return;
    }

    if (!Inventory->HasEmptySlot())
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory full for %s"), *User->GetName());
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Adding item to inventory"));
    
    // 인벤토리에 추가
    int32 SlotIndex = Inventory->GetFirstEmptySlotIndex();
    Inventory->ServerAddItem(this);
    
    // 슬롯 0에 추가되었으면 바로 장착
    if (SlotIndex == 0)
    {
        Inventory->SelectSlot(0);
        Inventory->AutoEquipItem(this);
    }
    
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    
    UE_LOG(LogTemp, Warning, TEXT("%s successfully picked up %s"), *User->GetName(), *ItemName.ToString());
}

void ACYItemBase::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                 bool bFromSweep, const FHitResult& SweepResult)
{
    // 플레이어가 E키를 눌러야 픽업되도록 하기 위해 여기서는 로그만 출력
    if (OtherActor && bCanPickup)
    {
        UE_LOG(LogTemp, Log, TEXT("Player near item: %s"), *ItemName.ToString());
    }
}

void ACYItemBase::ExecuteItemEffect(AActor* User)
{
    // 기본 구현은 비어있음 - 자식 클래스에서 오버라이드
}