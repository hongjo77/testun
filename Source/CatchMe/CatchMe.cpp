// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatchMe.h"
#include "CYGameplayTags.h" 
#include "Modules/ModuleManager.h"

// ✅ 로그 카테고리 정의
DEFINE_LOG_CATEGORY(LogCatchMe)

// ✅ 모듈 구현
void FCatchMeModule::StartupModule()
{
	// ✅ 모듈 로드 시점에서 GameplayTags 초기화 (가장 안전한 타이밍)
	UE_LOG(LogCatchMe, Warning, TEXT("🎮 Module: Initializing GameplayTags at module startup..."));
	FCYGameplayTags::InitializeNativeTags();
	UE_LOG(LogCatchMe, Warning, TEXT("🎮 Module: GameplayTags initialization completed"));
}

void FCatchMeModule::ShutdownModule()
{
	// 모듈 언로드 시 정리 작업
}

// ✅ 모듈 등록
IMPLEMENT_MODULE(FCatchMeModule, CatchMe)