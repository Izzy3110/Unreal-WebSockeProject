#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LMStudioUtils.generated.h"

/**
 * Utility functions for LM Studio integration.
 */
UCLASS()
class LMSTUDIOCONNECTOR_API ULMStudioUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Creates a JSON string formatted for LM Studio chat completions.
	 * @param Model The model identifier (e.g., "google/gemma-3-3b").
	 * @param SystemPrompt The system instruction for the AI.
	 * @param UserMessage The user's input message.
	 * @param Temperature Controls randomness (0.0 to 1.0).
	 * @return A formatted JSON string ready to be sent to the server.
	 */
	UFUNCTION(BlueprintPure, Category = "LMStudio|Utils")
	static FString CreateLMStudioRequestJSON(const FString& Model, const FString& SystemPrompt, const FString& UserMessage, float Temperature = 0.7f);
};
