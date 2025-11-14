#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LMStudioConnectorSettings.generated.h"

UCLASS(config=EditorPerProjectUserSettings, defaultconfig)
class LMSTUDIOCONNECTOR_API ULMStudioConnectorSettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(config, EditAnywhere, Category="LMStudio")
	FString ServerBaseURL = TEXT("http://localhost:1234/v1");

	UPROPERTY(config, EditAnywhere, Category="LMStudio")
	FString ModelName = TEXT("google/gemma-3-3b");

	UPROPERTY(config, EditAnywhere, Category="LMStudio")
	FString APIKey = TEXT("lm-studio");
};
