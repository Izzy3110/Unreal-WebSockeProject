#include "LMStudioConnector.h"
#include "LMStudioConnectorSettings.h"
#include "Engine/Engine.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Http.h"

ULMStudioConnector::ULMStudioConnector()
{
	ServerBaseURL = TEXT("http://localhost:3000/v1");
	ModelName = TEXT("google/gemma-3-3b");
	APIKey = TEXT("lm-studio");
}

void ULMStudioConnector::InitializeFromSettings()
{
	const ULMStudioConnectorSettings* Settings = GetDefault<ULMStudioConnectorSettings>();
	if (Settings)
	{
		ServerBaseURL = Settings->ServerBaseURL;
		ModelName = Settings->ModelName;
		APIKey = Settings->APIKey;
	}
}

void ULMStudioConnector::PrintSettings() const
{
	UE_LOG(LogTemp, Log, TEXT("LMStudioConnector Settings:"));
	UE_LOG(LogTemp, Log, TEXT("ServerBaseURL: %s"), *ServerBaseURL);
	UE_LOG(LogTemp, Log, TEXT("ModelName: %s"), *ModelName);
	UE_LOG(LogTemp, Log, TEXT("APIKey: %s"), *APIKey);
}

void ULMStudioConnector::TestServerConnection(FOnServerTestCompleted OnCompleted)
{
	if (ServerBaseURL.IsEmpty())
	{
		OnCompleted.ExecuteIfBound(false);
		return;
	}

	// Create HTTP request
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(ServerBaseURL);
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *APIKey));

	Request->OnProcessRequestComplete().BindLambda(
		[OnCompleted](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			bool bSuccess = bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200;
			OnCompleted.ExecuteIfBound(bSuccess);
		}
	);

	Request->ProcessRequest();
}
