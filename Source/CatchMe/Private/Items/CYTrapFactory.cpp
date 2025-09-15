#include "Items/CYTrapFactory.h"
#include "Items/CYTrapBase.h"
#include "Items/CYSlowTrap.h"
#include "Items/CYFreezeTrap.h"
#include "Items/CYDamageTrap.h"
#include "Items/CYItemBase.h"
#include "Engine/World.h"

// 정적 멤버 초기화
TMap<ETrapType, TSubclassOf<ACYTrapBase>> UCYTrapFactory::TrapClassMap;
bool UCYTrapFactory::bIsInitialized = false;

UCYTrapFactory::UCYTrapFactory()
{
    if (!bIsInitialized)
    {
        InitializeFactory();
    }
}

ACYTrapBase* UCYTrapFactory::CreateTrap(UWorld* World, ETrapType TrapType, const FVector& Location, 
                                        const FRotator& Rotation, AActor* Owner, APawn* Instigator)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("UCYTrapFactory::CreateTrap - World is null"));
        return nullptr;
    }

    if (!bIsInitialized)
    {
        InitializeFactory();
    }

    TSubclassOf<ACYTrapBase> TrapClass = GetTrapClass(TrapType);
    if (!TrapClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UCYTrapFactory::CreateTrap - No class registered for trap type %d"), 
               static_cast<int32>(TrapType));
        return nullptr;
    }

    // 트랩 스폰 파라미터 설정
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Instigator;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 트랩 생성
    ACYTrapBase* NewTrap = World->SpawnActor<ACYTrapBase>(TrapClass, Location, Rotation, SpawnParams);
    
    if (NewTrap)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ Created trap of type %s at location %s"), 
               *TrapClass->GetName(), *Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to spawn trap of type %s"), *TrapClass->GetName());
    }

    return NewTrap;
}

ACYTrapBase* UCYTrapFactory::CreateTrapFromItem(UWorld* World, ACYItemBase* SourceItem, 
                                                const FVector& Location, const FRotator& Rotation, 
                                                AActor* Owner, APawn* Instigator)
{
    if (!SourceItem)
    {
        UE_LOG(LogTemp, Error, TEXT("UCYTrapFactory::CreateTrapFromItem - SourceItem is null"));
        return nullptr;
    }

    // 아이템으로부터 트랩 타입 추론
    ETrapType TrapType = InferTrapTypeFromItem(SourceItem);
    
    UE_LOG(LogTemp, Warning, TEXT("🎯 Creating trap from item: %s -> TrapType: %d"), 
           *SourceItem->ItemName.ToString(), static_cast<int32>(TrapType));

    return CreateTrap(World, TrapType, Location, Rotation, Owner, Instigator);
}

void UCYTrapFactory::RegisterTrapClass(ETrapType TrapType, TSubclassOf<ACYTrapBase> TrapClass)
{
    if (!TrapClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UCYTrapFactory::RegisterTrapClass - TrapClass is null"));
        return;
    }

    TrapClassMap.Add(TrapType, TrapClass);
    UE_LOG(LogTemp, Log, TEXT("✅ Registered trap class %s for type %d"), 
           *TrapClass->GetName(), static_cast<int32>(TrapType));
}

TArray<ETrapType> UCYTrapFactory::GetRegisteredTrapTypes()
{
    TArray<ETrapType> RegisteredTypes;
    TrapClassMap.GetKeys(RegisteredTypes);
    return RegisteredTypes;
}

ETrapType UCYTrapFactory::InferTrapTypeFromItem(ACYItemBase* Item)
{
    if (!Item)
    {
        return ETrapType::Slow; // 기본값
    }

    FString ItemName = Item->ItemName.ToString().ToLower();
    
    // 아이템 이름으로 트랩 타입 추론
    if (ItemName.Contains(TEXT("slow")) || ItemName.Contains(TEXT("슬로우")))
    {
        return ETrapType::Slow;
    }
    else if (ItemName.Contains(TEXT("freeze")) || ItemName.Contains(TEXT("frost")) || 
             ItemName.Contains(TEXT("ice")) || ItemName.Contains(TEXT("프리즈")) || 
             ItemName.Contains(TEXT("얼음")))
    {
        return ETrapType::Freeze;
    }
    else if (ItemName.Contains(TEXT("damage")) || ItemName.Contains(TEXT("spike")) || 
             ItemName.Contains(TEXT("harm")) || ItemName.Contains(TEXT("데미지")) || 
             ItemName.Contains(TEXT("가시")))
    {
        return ETrapType::Damage;
    }
    else if (ItemName.Contains(TEXT("explosion")) || ItemName.Contains(TEXT("bomb")) || 
             ItemName.Contains(TEXT("폭발")) || ItemName.Contains(TEXT("폭탄")))
    {
        return ETrapType::Explosion;
    }

    // 기본값은 슬로우 트랩
    UE_LOG(LogTemp, Warning, TEXT("⚠️ Could not infer trap type from item name '%s', using Slow as default"), 
           *ItemName);
    return ETrapType::Slow;
}

TSubclassOf<ACYTrapBase> UCYTrapFactory::GetTrapClass(ETrapType TrapType)
{
    if (TSubclassOf<ACYTrapBase>* FoundClass = TrapClassMap.Find(TrapType))
    {
        return *FoundClass;
    }

    UE_LOG(LogTemp, Error, TEXT("UCYTrapFactory::GetTrapClass - No class found for trap type %d"), 
           static_cast<int32>(TrapType));
    return nullptr;
}

void UCYTrapFactory::InitializeFactory()
{
    if (bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("🏭 Initializing Trap Factory..."));

    // 기본 트랩 클래스들 등록
    RegisterTrapClass(ETrapType::Slow, ACYSlowTrap::StaticClass());
    RegisterTrapClass(ETrapType::Freeze, ACYFreezeTrap::StaticClass());
    RegisterTrapClass(ETrapType::Damage, ACYDamageTrap::StaticClass());
    
    // TODO: 폭발 트랩 클래스 추가 시 등록
    // RegisterTrapClass(ETrapType::Explosion, ACYExplosionTrap::StaticClass());

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("✅ Trap Factory initialized with %d trap types"), TrapClassMap.Num());
}