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
}