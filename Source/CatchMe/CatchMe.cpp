// Copyright Epic Games, Inc. All Rights Reserved.

#include "CatchMe.h"
#include "CYGameplayTags.h" 
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, CatchMe, "CatchMe" );

DEFINE_LOG_CATEGORY(LogCatchMe)

void FCatchMeModule::StartupModule()
{
	// 게임태그 초기화
	FCYGameplayTags::InitializeNativeTags();
}

void FCatchMeModule::ShutdownModule()
{
	// 정리 작업
}