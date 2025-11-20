#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "LMStudioRequest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLMStudioResponseDelegate, const FString&, ResponseContent, const FString&, ErrorMessage, bool, bSuccess);

/**
 * Async action to send requests to LM Studio
 */
UCLASS()
class LMSTUDIOCONNECTOR_API ULMStudioRequest : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Sends a JSON request to the LM Studio server.
	 * @param ServerURL The full URL to the endpoint (e.g., http://localhost:1234/v1/chat/completions)
	 * @param APIKey The API Key for authentication (optional)
	 * @param JsonContent The JSON body of the request
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "LMStudio")
	static ULMStudioRequest* SendLMStudioRequest(UObject* WorldContextObject, const FString& ServerURL, const FString& APIKey, const FString& JsonContent);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FLMStudioResponseDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FLMStudioResponseDelegate OnFailure;

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString ServerURL;
	FString APIKey;
	FString JsonContent;
};
