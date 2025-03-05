// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

D1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
D1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
D1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
D1GAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);

D1GAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
