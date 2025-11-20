#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "YamlWrapper.generated.h"

USTRUCT(BlueprintType)
struct FYamlKeyValue
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite) FString Key;
	UPROPERTY(BlueprintReadWrite) FString Value;
};

class FYamlWrapper
{
public:
	// Parse YAML text into key/value pairs for simple flat maps
	// Returns true on success.
	static bool ParseYamlToMap(const FString& YamlText, TMap<FString, FString>& OutMap);

	// Emit simple map into YAML text
	static bool EmitMapToYaml(const TMap<FString, FString>& Map, FString& OutYaml);
};
