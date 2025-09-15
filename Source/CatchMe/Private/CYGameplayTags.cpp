#include "CYGameplayTags.h"
#include "GameplayTagsManager.h"

FCYGameplayTags FCYGameplayTags::GameplayTags;

void FCYGameplayTags::InitializeNativeTags()
{
    // ✅ 이미 초기화되었는지 확인
    static bool bInitialized = false;
    if (bInitialized)
    {
        UE_LOG(LogTemp, Verbose, TEXT("GameplayTags already initialized"));
        return;
    }

    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // ✅ Manager가 준비되었는지 확인
    if (!Manager.ShouldImportTagsFromINI())
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayTagsManager not ready yet, deferring initialization"));
        return;
    }

    // ============ 어빌리티 태그 등록 ============
    GameplayTags.Ability_Weapon_Attack = Manager.AddNativeGameplayTag(
        FName("Ability.Weapon.Attack"),
        FString("무기 공격 어빌리티")
    );

    GameplayTags.Ability_Trap_Place = Manager.AddNativeGameplayTag(
        FName("Ability.Trap.Place"),
        FString("트랩 설치 어빌리티")
    );

    // ============ 아이템 태그 등록 ============
    GameplayTags.Item_Base = Manager.AddNativeGameplayTag(
        FName("Item.Base"),
        FString("기본 아이템")
    );

    GameplayTags.Item_Weapon = Manager.AddNativeGameplayTag(
        FName("Item.Weapon"),
        FString("무기 아이템")
    );

    GameplayTags.Item_Trap = Manager.AddNativeGameplayTag(
        FName("Item.Trap"),
        FString("트랩 아이템")
    );

    GameplayTags.Item_Consumable = Manager.AddNativeGameplayTag(
        FName("Item.Consumable"),
        FString("소모 아이템")
    );

    // ============ 상태 태그 등록 ============
    GameplayTags.State_Attacking = Manager.AddNativeGameplayTag(
        FName("State.Attacking"),
        FString("공격 중 상태")
    );

    GameplayTags.State_Stunned = Manager.AddNativeGameplayTag(
        FName("State.Stunned"),
        FString("기절 상태")
    );

    GameplayTags.State_Dead = Manager.AddNativeGameplayTag(
        FName("State.Dead"),
        FString("사망 상태")
    );

    // ============ 쿨다운 태그 등록 ============
    GameplayTags.Cooldown_Weapon_Attack = Manager.AddNativeGameplayTag(
        FName("Cooldown.Weapon.Attack"),
        FString("무기 공격 쿨다운")
    );

    GameplayTags.Cooldown_Trap_Place = Manager.AddNativeGameplayTag(
        FName("Cooldown.Trap.Place"),
        FString("트랩 설치 쿨다운")
    );

    // ============ 이벤트 태그 등록 ============
    GameplayTags.Event_Item_Use = Manager.AddNativeGameplayTag(
        FName("Event.Item.Use"),
        FString("아이템 사용 이벤트")
    );

    // ============ 데이터 태그 등록 ============
    GameplayTags.Data_Damage = Manager.AddNativeGameplayTag(
        FName("Data.Damage"),
        FString("데미지 데이터")
    );

    bInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("✅ GameplayTags successfully initialized"));
}