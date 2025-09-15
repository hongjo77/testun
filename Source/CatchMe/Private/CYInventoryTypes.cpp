// Source/CatchMe/Private/CYInventoryTypes.cpp
#include "CYInventoryTypes.h"

int32 UInventorySlotUtils::KeyToSlotIndex(int32 KeyNumber)
{
	if (KeyNumber >= 1 && KeyNumber <= 3)
	{
		// 무기 슬롯: 1번키=1000, 2번키=1001, 3번키=1002
		return WEAPON_SLOT_OFFSET + (KeyNumber - 1);
	}
	else if (KeyNumber >= 4 && KeyNumber <= 9)
	{
		// 아이템 슬롯: 4번키=0, 5번키=1, 6번키=2, 7번키=3, 8번키=4, 9번키=5
		return KeyNumber - 4;
	}
    
	// 잘못된 키 번호
	UE_LOG(LogTemp, Error, TEXT("❌ Invalid key number: %d"), KeyNumber);
	return -1;
}

bool UInventorySlotUtils::ParseSlotIndex(int32 UnifiedIndex, EInventorySlotType& OutSlotType, int32& OutLocalIndex)
{
	if (UnifiedIndex >= WEAPON_SLOT_OFFSET)
	{
		// 무기 슬롯
		OutSlotType = EInventorySlotType::Weapon;
		OutLocalIndex = UnifiedIndex - WEAPON_SLOT_OFFSET;
		return true;
	}
	else if (UnifiedIndex >= 0)
	{
		// 아이템 슬롯
		OutSlotType = EInventorySlotType::Item;
		OutLocalIndex = UnifiedIndex;
		return true;
	}
    
	// 잘못된 인덱스
	OutSlotType = EInventorySlotType::Item;
	OutLocalIndex = 0;
	return false;
}

int32 UInventorySlotUtils::MakeSlotIndex(EInventorySlotType SlotType, int32 LocalIndex)
{
	switch (SlotType)
	{
	case EInventorySlotType::Weapon:
		return WEAPON_SLOT_OFFSET + LocalIndex;
            
	case EInventorySlotType::Item:
		return LocalIndex;
            
	default:
		UE_LOG(LogTemp, Error, TEXT("❌ Unknown slot type"));
		return LocalIndex;
	}
}