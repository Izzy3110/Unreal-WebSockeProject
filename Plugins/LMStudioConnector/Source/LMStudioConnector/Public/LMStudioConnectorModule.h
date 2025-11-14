#pragma once

#include "Modules/ModuleManager.h"

class FLMStudioConnectorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
