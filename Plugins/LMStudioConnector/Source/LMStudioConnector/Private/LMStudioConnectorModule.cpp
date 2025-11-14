#include "LMStudioConnectorModule.h"
#include "LMStudioConnectorSettings.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FLMStudioConnectorModule"

void FLMStudioConnectorModule::StartupModule()
{
	// Register settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project",
			"Plugins",
			"LMStudio Connector",
			LOCTEXT("LMStudioConnectorSettingsName", "LMStudio Connector"),
			LOCTEXT("LMStudioConnectorSettingsDescription", "Settings for LMStudio Connector plugin"),
			GetMutableDefault<ULMStudioConnectorSettings>()
		);
	}
}

void FLMStudioConnectorModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "LMStudio Connector");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLMStudioConnectorModule, LMStudioConnector)
