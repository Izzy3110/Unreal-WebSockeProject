#include "YamlBlueprintLibrary.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"

// Third Party
#include "yaml-cpp/yaml.h"
#include <string>
#include <iostream>

// Helper function to convert YAML node to Unreal Json Value
TSharedPtr<FJsonValue> ConvertYamlNodeToJsonValue(const YAML::Node& Node)
{
	switch (Node.Type())
	{
	case YAML::NodeType::Null:
		return MakeShared<FJsonValueNull>();

	case YAML::NodeType::Scalar:
		{
			std::string ScalarValue = Node.as<std::string>();
			
			// Try to parse as boolean
			if (ScalarValue == "true") return MakeShared<FJsonValueBoolean>(true);
			if (ScalarValue == "false") return MakeShared<FJsonValueBoolean>(false);

			// Try to parse as number (double)
			// Note: This is a simple check, might need more robust parsing if strict typing is needed
			// But for JSON, strings are often safe. Let's try to be smart.
			try {
				size_t processed = 0;
				double d = std::stod(ScalarValue, &processed);
				if (processed == ScalarValue.length())
				{
					return MakeShared<FJsonValueNumber>(d);
				}
			} catch (...) {}

			// Default to string
			return MakeShared<FJsonValueString>(FString(ScalarValue.c_str()));
		}

	case YAML::NodeType::Sequence:
		{
			TArray<TSharedPtr<FJsonValue>> ArrayValues;
			for (auto It = Node.begin(); It != Node.end(); ++It)
			{
				ArrayValues.Add(ConvertYamlNodeToJsonValue(*It));
			}
			return MakeShared<FJsonValueArray>(ArrayValues);
		}

	case YAML::NodeType::Map:
		{
			TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
			for (auto It = Node.begin(); It != Node.end(); ++It)
			{
				std::string Key = It->first.as<std::string>();
				TSharedPtr<FJsonValue> Value = ConvertYamlNodeToJsonValue(It->second);
				JsonObject->SetField(FString(Key.c_str()), Value);
			}
			return MakeShared<FJsonValueObject>(JsonObject);
		}

	case YAML::NodeType::Undefined:
	default:
		return MakeShared<FJsonValueNull>();
	}
}

void UYamlBlueprintLibrary::LoadYamlFromFile(FString FilePath, FString& JsonOutput, bool& bSuccess, FString& ErrorMessage)
{
	bSuccess = false;
	JsonOutput = "";
	ErrorMessage = "";

	if (!FPaths::FileExists(FilePath))
	{
		ErrorMessage = FString::Printf(TEXT("File not found: %s"), *FilePath);
		return;
	}

	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		ErrorMessage = FString::Printf(TEXT("Failed to read file: %s"), *FilePath);
		return;
	}

	ParseYamlString(FileContent, JsonOutput, bSuccess, ErrorMessage);
}

void UYamlBlueprintLibrary::ParseYamlString(FString YamlString, FString& JsonOutput, bool& bSuccess, FString& ErrorMessage)
{
	bSuccess = false;
	JsonOutput = "";
	ErrorMessage = "";

	try
	{
		YAML::Node RootNode = YAML::Load(std::string(TCHAR_TO_UTF8(*YamlString)));
		
		TSharedPtr<FJsonValue> RootJsonValue = ConvertYamlNodeToJsonValue(RootNode);

		if (RootJsonValue.IsValid())
		{
			// Serialize to string
			TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonOutput);
			
			// FJsonSerializer::Serialize takes an Object or Array. 
			// If the root is a scalar, we might need to handle it, but valid JSON usually starts with Object or Array.
			// However, FJsonValue can be anything.
			// Let's handle Object and Array specifically, or wrap scalar?
			// Unreal's Serialize usually expects Object or Array.
			
			if (RootJsonValue->Type == EJson::Object)
			{
				FJsonSerializer::Serialize(RootJsonValue->AsObject().ToSharedRef(), Writer);
				bSuccess = true;
			}
			else if (RootJsonValue->Type == EJson::Array)
			{
				FJsonSerializer::Serialize(RootJsonValue->AsArray(), Writer);
				bSuccess = true;
			}
			else
			{
				// It's a scalar (string, number, bool, null)
				// Valid JSON can be just a value, but Unreal's serializer might want a container.
				// Let's just write the value manually if needed, or wrap it.
				// Actually, let's just use the Writer directly.
				// Writer->WriteValue? No, Writer->WriteObjectStart/End etc.
				
				// Let's just try to serialize it.
				// FJsonSerializer doesn't have a generic "Serialize Value".
				
				// Workaround: Wrap in an object if it's not one? No, that changes structure.
				// If it's a scalar, just return the string representation?
				// For now, let's assume the YAML root is a Map or Sequence.
				// If not, we'll error or handle it.
				
				ErrorMessage = "Root YAML node must be a Map or Sequence to be converted to valid JSON object/array.";
			}
		}
	}
	catch (const YAML::Exception& e)
	{
		ErrorMessage = FString::Printf(TEXT("YAML Parsing Error: %s"), UTF8_TO_TCHAR(e.what()));
	}
	catch (...)
	{
		ErrorMessage = "Unknown error during YAML parsing.";
	}
}



void UYamlBlueprintLibrary::GetValuesFromYamlSequence(FString FilePath, FString Key, TArray<FString>& Values, bool& bSuccess, FString& ErrorMessage)
{
	bSuccess = false;
	Values.Empty();
	ErrorMessage = "";

	if (!FPaths::FileExists(FilePath))
	{
		ErrorMessage = FString::Printf(TEXT("File not found: %s"), *FilePath);
		return;
	}

	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		ErrorMessage = FString::Printf(TEXT("Failed to read file: %s"), *FilePath);
		return;
	}

	GetValuesFromYamlString(FileContent, Key, Values, bSuccess, ErrorMessage);
}

void UYamlBlueprintLibrary::GetValuesFromYamlString(FString YamlString, FString Key, TArray<FString>& Values, bool& bSuccess, FString& ErrorMessage)
{
	bSuccess = false;
	Values.Empty();
	ErrorMessage = "";

	try
	{
		YAML::Node Root = YAML::Load(std::string(TCHAR_TO_UTF8(*YamlString)));

		if (!Root.IsSequence())
		{
			ErrorMessage = "Root YAML node must be a sequence.";
			return;
		}

		std::string KeyStr = std::string(TCHAR_TO_UTF8(*Key));

		for (const auto& Item : Root)
		{
			if (Item.IsMap() && Item[KeyStr])
			{
				YAML::Node ValueNode = Item[KeyStr];
				if (ValueNode.IsScalar())
				{
					std::string Val = ValueNode.as<std::string>();
					Values.Add(FString(UTF8_TO_TCHAR(Val.c_str())));
				}
				else
				{
					// Convert complex node to JSON string
					TSharedPtr<FJsonValue> JsonVal = ConvertYamlNodeToJsonValue(ValueNode);
					FString JsonString;
					TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
					
					if (JsonVal->Type == EJson::Array)
					{
						FJsonSerializer::Serialize(JsonVal->AsArray(), Writer);
						Values.Add(JsonString);
					}
					else if (JsonVal->Type == EJson::Object)
					{
						FJsonSerializer::Serialize(JsonVal->AsObject().ToSharedRef(), Writer);
						Values.Add(JsonString);
					}
					else
					{
						// Fallback for other types (should be covered by scalar check, but just in case)
						Values.Add("{}"); 
					}
				}
			}
		}

		bSuccess = true;
	}
	catch (const YAML::Exception& e)
	{
		ErrorMessage = FString::Printf(TEXT("YAML Parsing Error: %s"), UTF8_TO_TCHAR(e.what()));
	}
	catch (...)
	{
		ErrorMessage = "Unknown error during YAML parsing.";
	}
}

// Recursive helper for path traversal
void CollectValuesFromPath(const YAML::Node& Node, const TArray<FString>& PathSegments, int32 CurrentIndex, TArray<FString>& Results)
{
	if (!Node.IsDefined()) return;

	// If we reached the end of the path
	if (CurrentIndex >= PathSegments.Num())
	{
		if (Node.IsScalar())
		{
			std::string Val = Node.as<std::string>();
			Results.Add(FString(UTF8_TO_TCHAR(Val.c_str())));
		}
		else
		{
			// Convert complex node to JSON string
			TSharedPtr<FJsonValue> JsonVal = ConvertYamlNodeToJsonValue(Node);
			FString JsonString;
			TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
			
			if (JsonVal->Type == EJson::Array)
			{
				FJsonSerializer::Serialize(JsonVal->AsArray(), Writer);
				Results.Add(JsonString);
			}
			else if (JsonVal->Type == EJson::Object)
			{
				FJsonSerializer::Serialize(JsonVal->AsObject().ToSharedRef(), Writer);
				Results.Add(JsonString);
			}
			else
			{
				Results.Add("{}");
			}
		}
		return;
	}

	// If current node is a sequence, iterate through all items and apply the SAME path segment
	if (Node.IsSequence())
	{
		for (const auto& Item : Node)
		{
			CollectValuesFromPath(Item, PathSegments, CurrentIndex, Results);
		}
		return;
	}

	// If current node is a map, look for the key
	if (Node.IsMap())
	{
		std::string Key = std::string(TCHAR_TO_UTF8(*PathSegments[CurrentIndex]));
		if (Node[Key])
		{
			CollectValuesFromPath(Node[Key], PathSegments, CurrentIndex + 1, Results);
		}
	}
}

void UYamlBlueprintLibrary::GetValuesFromYamlPath(FString YamlString, FString Path, TArray<FString>& Values, bool& bSuccess, FString& ErrorMessage)
{
	bSuccess = false;
	Values.Empty();
	ErrorMessage = "";

	try
	{
		YAML::Node Root = YAML::Load(std::string(TCHAR_TO_UTF8(*YamlString)));

		TArray<FString> PathSegments;
		Path.ParseIntoArray(PathSegments, TEXT("."), true);

		CollectValuesFromPath(Root, PathSegments, 0, Values);

		bSuccess = true;
	}
	catch (const YAML::Exception& e)
	{
		ErrorMessage = FString::Printf(TEXT("YAML Parsing Error: %s"), UTF8_TO_TCHAR(e.what()));
	}
	catch (...)
	{
		ErrorMessage = "Unknown error during YAML parsing.";
	}
}

void UYamlBlueprintLibrary::GetValueFromYamlPath(FString YamlString, FString Path, FString& Value, bool& bSuccess, FString& ErrorMessage)
{
	bSuccess = false;
	Value = "";
	ErrorMessage = "";

	TArray<FString> Values;
	GetValuesFromYamlPath(YamlString, Path, Values, bSuccess, ErrorMessage);

	if (bSuccess && Values.Num() > 0)
	{
		Value = Values[0];
		bSuccess = true;
	}
	else
	{
		bSuccess = false; // Ensure false if no values found even if parsing succeeded
	}
}

// Helper to check if a node satisfies a specific path condition
bool CheckNodeCondition(const YAML::Node& Node, const TArray<FString>& PathSegments, int32 CurrentIndex, const FString& ExpectedValue)
{
	if (!Node.IsDefined()) return false;

	// If we reached the end of the path, check value
	if (CurrentIndex >= PathSegments.Num())
	{
		if (Node.IsScalar())
		{
			return FString(UTF8_TO_TCHAR(Node.as<std::string>().c_str())) == ExpectedValue;
		}
		return false; // Complex types don't match string value directly
	}

	// If sequence, we can't traverse down a specific path unless the path implies an index (which we don't support yet for filters)
	// OR we assume filters apply to the current context if it's a map.
	// If Node is a sequence, we can't really check "Path.SubPath" == Value on the sequence itself.
	// But usually filters diverge at a Map node.
	if (Node.IsSequence())
	{
		// If we encounter a sequence during filter check, it's ambiguous WHICH item to check.
		// For now, return false to be safe, or maybe check ANY?
		// Let's assume filters are designed to match 1:1 with the structure traversal.
		return false; 
	}

	if (Node.IsMap())
	{
		std::string Key = std::string(TCHAR_TO_UTF8(*PathSegments[CurrentIndex]));
		if (Node[Key])
		{
			return CheckNodeCondition(Node[Key], PathSegments, CurrentIndex + 1, ExpectedValue);
		}
	}

	return false;
}

void CollectValuesFromPathWithFilters(const YAML::Node& Node, const TArray<FString>& PathSegments, int32 CurrentIndex, const TMap<FString, FString>& Filters, TArray<FString>& Results)
{
	if (!Node.IsDefined()) return;

	// Check Filters at this level
	// We iterate through all filters. If a filter diverges from the current path at this node, we check it.
	// CRITICAL FIX: If we are at a Sequence and NOT at the end of the path, we must DEFER the check to the items.
	// The CurrentIndex refers to the property of the ITEMS, not the sequence itself.
	if (!Node.IsSequence() || CurrentIndex >= PathSegments.Num())
	{
		for (const auto& Filter : Filters)
		{
			FString FilterPath = Filter.Key;
			FString FilterValue = Filter.Value;
			
			TArray<FString> FilterSegments;
			FilterPath.ParseIntoArray(FilterSegments, TEXT("."), true);

			// If filter path is shorter than current depth, it's likely a parent filter (already checked) or invalid.
			// If filter path matches current path so far...
			bool bDivergesHere = false;
			if (FilterSegments.Num() > CurrentIndex)
			{
				// Check if all previous segments matched (they should have, if we are here, assuming we only check relevant filters)
				// Actually, we need to verify if this filter *applies* to this branch.
				// Simple heuristic: If the filter path starts with the SAME segments as the Target Path up to CurrentIndex,
				// AND the segment at CurrentIndex is DIFFERENT, then it diverges here.
				
				bool bPrefixMatches = true;
				for (int32 i = 0; i < CurrentIndex; i++)
				{
					if (i >= FilterSegments.Num() || FilterSegments[i] != PathSegments[i])
					{
						bPrefixMatches = false;
						break;
					}
				}

				if (bPrefixMatches)
				{
					// If we are at the end of Target Path (CurrentIndex == PathSegments.Num()), 
					// we are at the leaf. Any filter that matches prefix up to here and has more segments is a child filter.
					// If CurrentIndex < PathSegments.Num(), we are at an intermediate node.
					
					if (CurrentIndex < PathSegments.Num())
					{
						if (FilterSegments[CurrentIndex] != PathSegments[CurrentIndex])
						{
							bDivergesHere = true;
						}
					}
					else
					{
						// Target path ended. Filter continues. Diverges here (at the leaf of target).
						bDivergesHere = true;
					}
				}
			}

			if (bDivergesHere)
			{
				// Check the condition
				// We need to check Node[FilterSegments[CurrentIndex]]...
				// We can reuse CheckNodeCondition but we need to pass the *rest* of the filter path.
				
				// Construct rest of path
				TArray<FString> RestPath;
				for (int32 i = CurrentIndex; i < FilterSegments.Num(); i++)
				{
					RestPath.Add(FilterSegments[i]);
				}
				
				if (!CheckNodeCondition(Node, RestPath, 0, FilterValue))
				{
					return; // Filter failed, prune branch
				}
			}
		}
	}

	// If we reached the end of the path
	if (CurrentIndex >= PathSegments.Num())
	{
		if (Node.IsScalar())
		{
			std::string Val = Node.as<std::string>();
			Results.Add(FString(UTF8_TO_TCHAR(Val.c_str())));
		}
		else
		{
			// Convert complex node to JSON string
			TSharedPtr<FJsonValue> JsonVal = ConvertYamlNodeToJsonValue(Node);
			FString JsonString;
			TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
			
			if (JsonVal->Type == EJson::Array)
			{
				FJsonSerializer::Serialize(JsonVal->AsArray(), Writer);
				Results.Add(JsonString);
			}
			else if (JsonVal->Type == EJson::Object)
			{
				FJsonSerializer::Serialize(JsonVal->AsObject().ToSharedRef(), Writer);
				Results.Add(JsonString);
			}
			else
			{
				Results.Add("{}");
			}
		}
		return;
	}

	// If current node is a sequence, iterate through all items
	if (Node.IsSequence())
	{
		for (const auto& Item : Node)
		{
			CollectValuesFromPathWithFilters(Item, PathSegments, CurrentIndex, Filters, Results);
		}
		return;
	}

	// If current node is a map, look for the key
	if (Node.IsMap())
	{
		std::string Key = std::string(TCHAR_TO_UTF8(*PathSegments[CurrentIndex]));
		if (Node[Key])
		{
			CollectValuesFromPathWithFilters(Node[Key], PathSegments, CurrentIndex + 1, Filters, Results);
		}
	}
}

void UYamlBlueprintLibrary::GetValuesFromYamlPathWithFilters(FString YamlString, FString Path, TMap<FString, FString> Filters, TArray<FString>& Values, bool& bSuccess, FString& ErrorMessage)
{
	bSuccess = false;
	Values.Empty();
	ErrorMessage = "";

	try
	{
		YAML::Node Root = YAML::Load(std::string(TCHAR_TO_UTF8(*YamlString)));

		TArray<FString> PathSegments;
		Path.ParseIntoArray(PathSegments, TEXT("."), true);

		CollectValuesFromPathWithFilters(Root, PathSegments, 0, Filters, Values);

		bSuccess = true;
	}
	catch (const YAML::Exception& e)
	{
		ErrorMessage = FString::Printf(TEXT("YAML Parsing Error: %s"), UTF8_TO_TCHAR(e.what()));
	}
	catch (...)
	{
		ErrorMessage = "Unknown error during YAML parsing.";
	}
}

void UYamlBlueprintLibrary::GetValueFromYamlPathWithFilters(FString YamlString, FString Path, TMap<FString, FString> Filters, FString& Value, bool& bSuccess, FString& ErrorMessage)
{
	bSuccess = false;
	Value = "";
	ErrorMessage = "";

	TArray<FString> Values;
	GetValuesFromYamlPathWithFilters(YamlString, Path, Filters, Values, bSuccess, ErrorMessage);

	if (bSuccess && Values.Num() > 0)
	{
		Value = Values[0];
		bSuccess = true;
	}
	else
	{
		bSuccess = false;
	}
}
