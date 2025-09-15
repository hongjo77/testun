#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Items/CYTrapData.h"
#include "CYTrapFactory.generated.h"

class ACYTrapBase;
class ACYItemBase;
class UWorld;

/**
 * 팩토리 패턴을 사용한 트랩 생성 클래스
 * 확장성과 유지보수성을 위해 설계됨
 */
UCLASS(BlueprintType)
class CATCHME_API UCYTrapFactory : public UObject
{
	GENERATED_BODY()

public:
	UCYTrapFactory();

	// 정적 팩토리 메서드
	UFUNCTION(BlueprintCallable, Category = "Trap Factory", CallInEditor)
	static ACYTrapBase* CreateTrap(
		UWorld* World,
		ETrapType TrapType,
		const FVector& Location,
		const FRotator& Rotation,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr
	);

	// 아이템으로부터 트랩 생성 (기존 호환성)
	UFUNCTION(BlueprintCallable, Category = "Trap Factory")
	static ACYTrapBase* CreateTrapFromItem(
		UWorld* World,
		ACYItemBase* SourceItem,
		const FVector& Location,
		const FRotator& Rotation,
		AActor* Owner = nullptr,
		APawn* Instigator = nullptr
	);

	// 트랩 타입 등록 (모듈화된 확장)
	UFUNCTION(BlueprintCallable, Category = "Trap Factory")
	static void RegisterTrapClass(ETrapType TrapType, TSubclassOf<ACYTrapBase> TrapClass);

	// 등록된 트랩 타입들 조회
	UFUNCTION(BlueprintCallable, Category = "Trap Factory")
	static TArray<ETrapType> GetRegisteredTrapTypes();

protected:
	// 트랩 타입별 클래스 매핑
	static TMap<ETrapType, TSubclassOf<ACYTrapBase>> TrapClassMap;

	// 아이템 이름으로부터 트랩 타입 추론
	static ETrapType InferTrapTypeFromItem(ACYItemBase* Item);

	// 트랩 클래스 가져오기
	static TSubclassOf<ACYTrapBase> GetTrapClass(ETrapType TrapType);

	// 팩토리 초기화 (기본 트랩 클래스들 등록)
	static void InitializeFactory();

	// 팩토리 초기화 여부
	static bool bIsInitialized;
};