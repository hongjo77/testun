// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogCatchMe, Log, All);

/** 
 * CatchMe 게임 모듈 클래스
 * GameplayTags 초기화 등 모듈 수준의 초기화를 담당
 */
class FCatchMeModule : public IModuleInterface
{
public:
	/** Called when the module is loaded */
	virtual void StartupModule() override;
    
	/** Called when the module is unloaded */
	virtual void ShutdownModule() override;
};