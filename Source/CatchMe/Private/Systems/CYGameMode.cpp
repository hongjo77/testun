#include "Systems/CYGameMode.h"

#include "CYGameplayTags.h"
#include "Player/CYPlayerController.h"
#include "Player/CYPlayerCharacter.h"

ACYGameMode::ACYGameMode()
{
	DefaultPawnClass = ACYPlayerCharacter::StaticClass();
	PlayerControllerClass = ACYPlayerController::StaticClass();
	
}

void ACYGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// ✅ 게임 시작 시 한 번만 GameplayTags 초기화
	UE_LOG(LogTemp, Warning, TEXT("🎮 GameMode: Initializing GameplayTags..."));
	FCYGameplayTags::InitializeNativeTags();
	UE_LOG(LogTemp, Warning, TEXT("🎮 GameMode: GameplayTags initialization completed"));
}