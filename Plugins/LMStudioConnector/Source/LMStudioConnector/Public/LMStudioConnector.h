#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LMStudioConnector.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnServerTestCompleted, bool, bSuccess);

/**
 * Minimal LMStudio Connector class.
 */
UCLASS(Blueprintable)
class LMSTUDIOCONNECTOR_API ULMStudioConnector : public UObject
{
	GENERATED_BODY()

public:

	ULMStudioConnector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LMStudio")
	FString ServerBaseURL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LMStudio")
	FString ModelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LMStudio")
	FString APIKey;

	UFUNCTION(BlueprintCallable, Category="LMStudio")
	void InitializeFromSettings();

	UFUNCTION(BlueprintCallable, Category="LMStudio")
	void PrintSettings() const;

	/** Test the server connection via GET request */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="LMStudio")
	void TestServerConnection(FOnServerTestCompleted OnCompleted);
};
