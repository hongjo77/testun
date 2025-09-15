// Source/CatchMe/Public/CYInventoryTypes.h
#pragma once

#include "CoreMinimal.h"
#include "CYInventoryTypes.generated.h"

/**
 * 인벤토리 슬롯 타입
 */
UENUM(BlueprintType)
enum class EInventorySlotType : uint8
{
	Item        UMETA(DisplayName = "Item Slot"),     // 일반 아이템 슬롯 (4-9번 키)
	Weapon      UMETA(DisplayName = "Weapon Slot")    // 무기 슬롯 (1-3번 키)
};

/**
 * 슬롯 유틸리티 클래스 - 기존 하드코딩 방식을 개선
 */
UCLASS(BlueprintType)
class CATCHME_API UInventorySlotUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 기존 시스템 호환: 키 번호 → 통합 슬롯 인덱스
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static int32 KeyToSlotIndex(int32 KeyNumber);

	// 기존 시스템 호환: 통합 슬롯 인덱스 → 슬롯 타입 + 로컬 인덱스
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static bool ParseSlotIndex(int32 UnifiedIndex, EInventorySlotType& OutSlotType, int32& OutLocalIndex);

	// 슬롯 타입과 로컬 인덱스 → 통합 슬롯 인덱스
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static int32 MakeSlotIndex(EInventorySlotType SlotType, int32 LocalIndex);

	// 타입 체크
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static bool IsWeaponSlot(int32 UnifiedIndex) { return UnifiedIndex >= WEAPON_SLOT_OFFSET; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static bool IsItemSlot(int32 UnifiedIndex) { return UnifiedIndex < WEAPON_SLOT_OFFSET; }

private:
	// 기존 하드코딩 방식 유지 (호환성)
	static constexpr int32 WEAPON_SLOT_OFFSET = 1000;
};