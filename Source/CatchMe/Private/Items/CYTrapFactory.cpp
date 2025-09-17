// CYTrapFactory.cpp - 개선된 팩토리 시스템
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

    // ✅ 트랩 생성 및 즉시 시각적 초기화
    ACYTrapBase* NewTrap = World->SpawnActor<ACYTrapBase>(TrapClass, Location, Rotation, SpawnParams);
    
    if (NewTrap)
    {
        // ✅ 플레이어가 설치한 트랩으로 변환
        NewTrap->ConvertToPlayerPlacedTrap(Owner);
        
        UE_LOG(LogTemp, Warning, TEXT("✅ Created PLAYER PLACED trap: %s at location %s"), 
               *GetTrapTypeName(TrapType), *Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to spawn trap of type %s"), 
               *GetTrapTypeName(TrapType));
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
    
    UE_LOG(LogTemp, Warning, TEXT("🎯 Creating trap from item: %s -> %s"), 
           *SourceItem->ItemName.ToString(), *GetTrapTypeName(TrapType));

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
    UE_LOG(LogTemp, Warning, TEXT("✅ Registered trap class %s for type %s"), 
           *TrapClass->GetName(), *GetTrapTypeName(TrapType));
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
    
    UE_LOG(LogTemp, Warning, TEXT("🏭 InferTrapTypeFromItem: '%s'"), *ItemName);
    
    // ✅ 개선된 추론 로직
    if (ItemName.Contains(TEXT("freeze")) || ItemName.Contains(TEXT("frost")) || 
        ItemName.Contains(TEXT("ice")) || ItemName.Contains(TEXT("프리즈")) || 
        ItemName.Contains(TEXT("얼음")) || ItemName.Contains(TEXT("test trap")))
    {
        UE_LOG(LogTemp, Warning, TEXT("🏭 -> Freeze Trap"));
        return ETrapType::Freeze;
    }
    else if (ItemName.Contains(TEXT("damage")) || ItemName.Contains(TEXT("spike")) || 
             ItemName.Contains(TEXT("harm")) || ItemName.Contains(TEXT("데미지")) || 
             ItemName.Contains(TEXT("가시")) || ItemName.Contains(TEXT("hurt")))
    {
        UE_LOG(LogTemp, Warning, TEXT("🏭 -> Damage Trap"));
        return ETrapType::Damage;
    }
    else if (ItemName.Contains(TEXT("slow")) || ItemName.Contains(TEXT("슬로우")) ||
             ItemName.Contains(TEXT("mud")) || ItemName.Contains(TEXT("늪")))
    {
        UE_LOG(LogTemp, Warning, TEXT("🏭 -> Slow Trap"));
        return ETrapType::Slow;
    }
    else if (ItemName.Contains(TEXT("explosion")) || ItemName.Contains(TEXT("bomb")) || 
             ItemName.Contains(TEXT("폭발")) || ItemName.Contains(TEXT("폭탄")))
    {
        UE_LOG(LogTemp, Warning, TEXT("🏭 -> Explosion Trap (Not implemented yet)"));
        return ETrapType::Explosion;
    }

    // ✅ 기본값: Slow Trap
    UE_LOG(LogTemp, Warning, TEXT("⚠️ Could not infer trap type from '%s', using Slow as default"), 
           *ItemName);
    return ETrapType::Slow;
}

TSubclassOf<ACYTrapBase> UCYTrapFactory::GetTrapClass(ETrapType TrapType)
{
    if (TSubclassOf<ACYTrapBase>* FoundClass = TrapClassMap.Find(TrapType))
    {
        return *FoundClass;
    }

    UE_LOG(LogTemp, Error, TEXT("UCYTrapFactory::GetTrapClass - No class found for trap type %s"), 
           *GetTrapTypeName(TrapType));
    return nullptr;
}

FString UCYTrapFactory::GetTrapTypeName(ETrapType TrapType)
{
    switch (TrapType)
    {
        case ETrapType::Slow: return TEXT("SlowTrap");
        case ETrapType::Freeze: return TEXT("FreezeTrap");
        case ETrapType::Damage: return TEXT("DamageTrap");
        case ETrapType::Explosion: return TEXT("ExplosionTrap");
        default: return TEXT("UnknownTrap");
    }
}

void UCYTrapFactory::InitializeFactory()
{
    if (bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("🏭 Initializing Trap Factory..."));

    // ✅ 기본 트랩 클래스들 등록
    RegisterTrapClass(ETrapType::Slow, ACYSlowTrap::StaticClass());
    RegisterTrapClass(ETrapType::Freeze, ACYFreezeTrap::StaticClass());
    RegisterTrapClass(ETrapType::Damage, ACYDamageTrap::StaticClass());
    
    // TODO: 폭발 트랩 클래스 추가 시 등록
    // RegisterTrapClass(ETrapType::Explosion, ACYExplosionTrap::StaticClass());

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("✅ Trap Factory initialized with %d trap types"), TrapClassMap.Num());
    
    // ✅ 등록된 트랩들 로그 출력
    for (const auto& TrapPair : TrapClassMap)
    {
        UE_LOG(LogTemp, Log, TEXT("   - %s: %s"), 
               *GetTrapTypeName(TrapPair.Key), 
               *TrapPair.Value->GetName());
    }
}