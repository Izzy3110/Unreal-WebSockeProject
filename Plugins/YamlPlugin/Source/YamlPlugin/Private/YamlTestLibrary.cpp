#include "YamlTestLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// Third Party
#include "yaml-cpp/yaml.h"
#include <string>
#include <vector>

void UYamlTestLibrary::TestPromptsYaml(FString FilePath, bool& bSuccess, FString& Log)
{
	bSuccess = false;
	Log = "";

	if (!FPaths::FileExists(FilePath))
	{
		Log = FString::Printf(TEXT("File not found: %s"), *FilePath);
		return;
	}

	try
	{
		YAML::Node Root = YAML::LoadFile(std::string(TCHAR_TO_UTF8(*FilePath)));

		if (!Root.IsSequence())
		{
			Log += TEXT("Error: Root must be a sequence.\n");
			return;
		}

		bool bAllValid = true;
		int Index = 0;

		for (const auto& Item : Root)
		{
			if (!Item.IsMap())
			{
				Log += FString::Printf(TEXT("Error: Item %d is not a map.\n"), Index);
				bAllValid = false;
				continue;
			}

			if (!Item["model"])
			{
				Log += FString::Printf(TEXT("Error: Item %d missing 'model'.\n"), Index);
				bAllValid = false;
			}
			if (!Item["input"])
			{
				Log += FString::Printf(TEXT("Error: Item %d missing 'input'.\n"), Index);
				bAllValid = false;
			}
			// 'reasoning' seems optional or at least present in example
			if (Item["reasoning"] && !Item["reasoning"].IsMap())
			{
				Log += FString::Printf(TEXT("Error: Item %d 'reasoning' is not a map.\n"), Index);
				bAllValid = false;
			}

			Index++;
		}

		if (bAllValid)
		{
			Log += TEXT("Validation Successful. All items match schema.\n");
			bSuccess = true;
		}
	}
	catch (const YAML::Exception& e)
	{
		Log = FString::Printf(TEXT("YAML Exception: %s"), UTF8_TO_TCHAR(e.what()));
	}
	catch (...)
	{
		Log = TEXT("Unknown error during validation.");
	}
}

void UYamlTestLibrary::TestResponsesYaml(FString FilePath, bool& bSuccess, FString& Log)
{
	bSuccess = false;
	Log = "";

	if (!FPaths::FileExists(FilePath))
	{
		Log = FString::Printf(TEXT("File not found: %s"), *FilePath);
		return;
	}

	try
	{
		YAML::Node Root = YAML::LoadFile(std::string(TCHAR_TO_UTF8(*FilePath)));

		if (!Root.IsSequence())
		{
			Log += TEXT("Error: Root must be a sequence.\n");
			return;
		}

		bool bAllValid = true;
		int Index = 0;

		for (const auto& Item : Root)
		{
			if (!Item.IsMap())
			{
				Log += FString::Printf(TEXT("Error: Item %d is not a map.\n"), Index);
				bAllValid = false;
				continue;
			}

			// Check required fields based on example
			const std::vector<std::string> RequiredFields = { "id", "object", "created_at", "status", "model", "output", "usage" };
			
			for (const auto& Field : RequiredFields)
			{
				if (!Item[Field])
				{
					Log += FString::Printf(TEXT("Error: Item %d missing '%s'.\n"), Index, UTF8_TO_TCHAR(Field.c_str()));
					bAllValid = false;
				}
			}

			if (Item["output"] && !Item["output"].IsSequence())
			{
				Log += FString::Printf(TEXT("Error: Item %d 'output' is not a sequence.\n"), Index);
				bAllValid = false;
			}

			if (Item["usage"] && !Item["usage"].IsMap())
			{
				Log += FString::Printf(TEXT("Error: Item %d 'usage' is not a map.\n"), Index);
				bAllValid = false;
			}

			Index++;
		}

		if (bAllValid)
		{
			Log += TEXT("Validation Successful. All items match schema.\n");
			bSuccess = true;
		}
	}
	catch (const YAML::Exception& e)
	{
		Log = FString::Printf(TEXT("YAML Exception: %s"), UTF8_TO_TCHAR(e.what()));
	}
	catch (...)
	{
		Log = TEXT("Unknown error during validation.");
	}
}
