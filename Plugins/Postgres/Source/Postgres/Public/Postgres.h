// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Central module header. Keep heavy includes out of here.
class FPostgresModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

// Ensures libpq and its Windows dependencies are loaded. Safe to call multiple times.
POSTGRES_API bool Postgres_EnsureLibpqLoaded();