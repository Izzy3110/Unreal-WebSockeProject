#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "YamlIO.generated.h"

UCLASS()
class UYamlIO : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="YAML")
	static bool WriteMapToYamlFile(const FString& AbsolutePath, const TMap<FString, FString>& Map);

	UFUNCTION(BlueprintCallable, Category="YAML")
	static bool ReadYamlFileToMap(const FString& AbsolutePath, TMap<FString, FString>& OutMap);
};
