#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "YamlTestLibrary.generated.h"

/**
 * Library for testing specific YAML schemas.
 */
UCLASS()
class YAMLPLUGIN_API UYamlTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Validates the structure of the Tests.Prompts.yaml file.
	 * 
	 * @param FilePath - Absolute path to the YAML file.
	 * @param bSuccess - True if the file is valid according to the schema.
	 * @param Log - Validation log or error messages.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML|Testing")
	static void TestPromptsYaml(FString FilePath, bool& bSuccess, FString& Log);

	/**
	 * Validates the structure of the Tests.Responses.yaml file.
	 * 
	 * @param FilePath - Absolute path to the YAML file.
	 * @param bSuccess - True if the file is valid according to the schema.
	 * @param Log - Validation log or error messages.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML|Testing")
	static void TestResponsesYaml(FString FilePath, bool& bSuccess, FString& Log);
};
