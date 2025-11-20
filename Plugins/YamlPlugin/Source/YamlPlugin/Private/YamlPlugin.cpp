#include "YamlPlugin.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogYamlPlugin, Log, All);

void FYamlPluginModule::StartupModule()
{
	UE_LOG(LogYamlPlugin, Log, TEXT("YamlPlugin starting up."));
	// optional: initialize third-party stuff here
}

void FYamlPluginModule::ShutdownModule()
{
	UE_LOG(LogYamlPlugin, Log, TEXT("YamlPlugin shutting down."));
}

IMPLEMENT_MODULE(FYamlPluginModule, YamlPlugin)
