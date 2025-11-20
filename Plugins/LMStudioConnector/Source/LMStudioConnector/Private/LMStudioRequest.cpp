#include "LMStudioRequest.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

ULMStudioRequest* ULMStudioRequest::SendLMStudioRequest(UObject* WorldContextObject, const FString& ServerURL, const FString& APIKey, const FString& JsonContent)
{
	ULMStudioRequest* BlueprintNode = NewObject<ULMStudioRequest>();
	BlueprintNode->ServerURL = ServerURL;
	BlueprintNode->APIKey = APIKey;
	BlueprintNode->JsonContent = JsonContent;
	return BlueprintNode;
}

void ULMStudioRequest::Activate()
{
	if (ServerURL.IsEmpty())
	{
		OnFailure.Broadcast(TEXT(""), TEXT("Server URL is empty"), false);
		return;
	}

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerURL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	
	if (!APIKey.IsEmpty())
	{
		Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *APIKey));
	}

	Request->SetContentAsString(JsonContent);

	Request->OnProcessRequestComplete().BindUObject(this, &ULMStudioRequest::OnResponseReceived);
	Request->ProcessRequest();
}

void ULMStudioRequest::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		if (Response->GetResponseCode() >= 200 && Response->GetResponseCode() < 300)
		{
			OnSuccess.Broadcast(Response->GetContentAsString(), TEXT(""), true);
		}
		else
		{
			OnFailure.Broadcast(Response->GetContentAsString(), FString::Printf(TEXT("HTTP Error: %d"), Response->GetResponseCode()), false);
		}
	}
	else
	{
		OnFailure.Broadcast(TEXT(""), TEXT("Connection failed"), false);
	}
}
