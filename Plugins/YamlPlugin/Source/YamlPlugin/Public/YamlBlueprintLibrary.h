#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "YamlBlueprintLibrary.generated.h"

/**
 * Library for exposing YAML functionality to Blueprints.
 */
UCLASS()
class YAMLPLUGIN_API UYamlBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Loads a YAML file and converts it to a JSON string.
	 * 
	 * @param FilePath - Absolute path to the YAML file.
	 * @param JsonOutput - The resulting JSON string.
	 * @param bSuccess - True if the file was loaded and parsed successfully.
	 * @param ErrorMessage - Error message if something went wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML")
	static void LoadYamlFromFile(FString FilePath, FString& JsonOutput, bool& bSuccess, FString& ErrorMessage);

	/**
	 * Parses a YAML string and converts it to a JSON string.
	 * 
	 * @param YamlString - The YAML content string.
	 * @param JsonOutput - The resulting JSON string.
	 * @param bSuccess - True if the string was parsed successfully.
	 * @param ErrorMessage - Error message if something went wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML")
	static void ParseYamlString(FString YamlString, FString& JsonOutput, bool& bSuccess, FString& ErrorMessage);

	/**
	 * Extracts all values for a specific key from a YAML sequence of maps.
	 * Useful for extracting lists of 'input' or 'model' from files like Tests.Prompts.yaml.
	 * 
	 * @param FilePath - Absolute path to the YAML file.
	 * @param Key - The key to look for in each item of the sequence.
	 * @param Values - Array of string values found for the key.
	 * @param bSuccess - True if the file was loaded and is a sequence.
	 * @param ErrorMessage - Error message if something went wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML")
	static void GetValuesFromYamlSequence(FString FilePath, FString Key, TArray<FString>& Values, bool& bSuccess, FString& ErrorMessage);

	/**
	 * Extracts all values for a specific key from a YAML/JSON string sequence of maps.
	 * 
	 * @param YamlString - The YAML or JSON content string.
	 * @param Key - The key to look for in each item of the sequence.
	 * @param Values - Array of string values found for the key.
	 * @param bSuccess - True if the string was parsed and is a sequence.
	 * @param ErrorMessage - Error message if something went wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML")
	static void GetValuesFromYamlString(FString YamlString, FString Key, TArray<FString>& Values, bool& bSuccess, FString& ErrorMessage);

	/**
	 * Extracts values from a YAML/JSON string using a dot-separated path (e.g., "output.content.text").
	 * Automatically iterates through sequences found in the path.
	 * 
	 * @param YamlString - The YAML or JSON content string.
	 * @param Path - Dot-separated path to the value.
	 * @param Values - Array of string values found.
	 * @param bSuccess - True if parsing was successful.
	 * @param ErrorMessage - Error message if something went wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML")
	static void GetValuesFromYamlPath(FString YamlString, FString Path, TArray<FString>& Values, bool& bSuccess, FString& ErrorMessage);

	/**
	 * Extracts a SINGLE value from a YAML/JSON string using a dot-separated path.
	 * Returns the FIRST value found if multiple exist.
	 * 
	 * @param YamlString - The YAML or JSON content string.
	 * @param Path - Dot-separated path to the value.
	 * @param Value - The first string value found.
	 * @param bSuccess - True if at least one value was found.
	 * @param ErrorMessage - Error message if something went wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML")
	static void GetValueFromYamlPath(FString YamlString, FString Path, FString& Value, bool& bSuccess, FString& ErrorMessage);

	/**
	 * Extracts values from a YAML/JSON string using a path, but only if they satisfy the provided filters.
	 * Filters are Key-Value pairs where the Key is a path (relative to root) and Value is the expected string value.
	 * 
	 * Example: Path="output.content.text", Filters={"output.content.type": "output_text"}
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML")
	static void GetValuesFromYamlPathWithFilters(FString YamlString, FString Path, TMap<FString, FString> Filters, TArray<FString>& Values, bool& bSuccess, FString& ErrorMessage);

	/**
	 * Extracts a SINGLE value from a YAML/JSON string using a path with filters.
	 * Returns the first value found that satisfies the filters.
	 */
	UFUNCTION(BlueprintCallable, Category = "YAML")
	static void GetValueFromYamlPathWithFilters(FString YamlString, FString Path, TMap<FString, FString> Filters, FString& Value, bool& bSuccess, FString& ErrorMessage);
};
