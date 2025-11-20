#include "YamlIO.h"
#include "YamlWrapper.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

bool UYamlIO::WriteMapToYamlFile(const FString& AbsolutePath, const TMap<FString, FString>& Map)
{
	FString OutYaml;
	if (!FYamlWrapper::EmitMapToYaml(Map, OutYaml)) return false;
	return FFileHelper::SaveStringToFile(OutYaml, *AbsolutePath);
}

bool UYamlIO::ReadYamlFileToMap(const FString& AbsolutePath, TMap<FString, FString>& OutMap)
{
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *AbsolutePath)) return false;
	return FYamlWrapper::ParseYamlToMap(Content, OutMap);
}
