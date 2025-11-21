#include "LMStudioRequest.h"
#include "LMStudioRequest.h"
#include "LMStudioUtils.h"
#include "LMStudioConnectorSettings.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

static FString LogFilePath;

ULMStudioRequest* ULMStudioRequest::SendLMStudioRequest(UObject* WorldContextObject, const FString& ServerURL, const FString& APIKey, const FString& JsonContent, const TArray<FString>& LastResponses)
{
	ULMStudioRequest* BlueprintNode = NewObject<ULMStudioRequest>();
	
	// Use settings as fallback
	const ULMStudioConnectorSettings* Settings = GetDefault<ULMStudioConnectorSettings>();
	
	FString FinalURL = ServerURL.IsEmpty() ? Settings->ServerBaseURL : ServerURL;
	
	// Ensure URL ends with /chat/completions or /responses
	if (!FinalURL.EndsWith(TEXT("/chat/completions")) && !FinalURL.EndsWith(TEXT("/responses")))
	{
		if (!FinalURL.EndsWith(TEXT("/")))
		{
			FinalURL += TEXT("/");
		}
		FinalURL += TEXT("chat/completions");
	}
	
	BlueprintNode->ServerURL = FinalURL;
	BlueprintNode->APIKey = APIKey.IsEmpty() ? Settings->APIKey : APIKey;
	BlueprintNode->JsonContent = JsonContent;
	BlueprintNode->LastResponses = LastResponses;
	return BlueprintNode;
}

ULMStudioRequest* ULMStudioRequest::SendLMStudioRequestSimple(UObject* WorldContextObject, const FString& ServerURL, const FString& APIKey, const FString& PromptPlainText, const FString& ModelName, const TArray<FString>& LastResponses)
{
	ULMStudioRequest* BlueprintNode = NewObject<ULMStudioRequest>();
	
	// Use settings as fallback
	const ULMStudioConnectorSettings* Settings = GetDefault<ULMStudioConnectorSettings>();
	
	FString FinalURL = ServerURL.IsEmpty() ? Settings->ServerBaseURL : ServerURL;
	
	// Ensure URL ends with /chat/completions or /responses
	if (!FinalURL.EndsWith(TEXT("/chat/completions")) && !FinalURL.EndsWith(TEXT("/responses")))
	{
		if (!FinalURL.EndsWith(TEXT("/")))
		{
			FinalURL += TEXT("/");
		}
		FinalURL += TEXT("chat/completions");
	}
	
	BlueprintNode->ServerURL = FinalURL;
	BlueprintNode->APIKey = APIKey.IsEmpty() ? Settings->APIKey : APIKey;
	BlueprintNode->LastResponses = LastResponses;
	
	FString TargetModel = ModelName.IsEmpty() ? Settings->ModelName : ModelName;

	// Create default JSON request
	// Note: We use a default system prompt and temperature here. 
	// If more control is needed, users should use the advanced node.
	FString SystemPrompt = TEXT("You are a helpful assistant.");
	BlueprintNode->JsonContent = ULMStudioUtils::CreateLMStudioRequestJSON(TargetModel, SystemPrompt, PromptPlainText);
	
	return BlueprintNode;
}

void ULMStudioRequest::Activate()
{
	StartTime = FPlatformTime::Seconds();

	// Initialize LogFilePath if empty
	if (LogFilePath.IsEmpty())
	{
		FString AppDataPath = FPlatformMisc::GetEnvironmentVariable(TEXT("APPDATA"));
		FString LogDir = FPaths::Combine(AppDataPath, TEXT("WebSockeProject"), TEXT("Logs"), TEXT("LMStudio"));
		
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DirectoryExists(*LogDir))
		{
			PlatformFile.CreateDirectoryTree(*LogDir);
		}

		FString Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
		LogFilePath = FPaths::Combine(LogDir, FString::Printf(TEXT("requests-%s.log"), *Timestamp));
	}

	if (ServerURL.IsEmpty())
	{
		OnFailure.Broadcast(TEXT(""), TEXT("Server URL is empty"), false);
		SetReadyToDestroy();
		return;
	}

	// Parse JSON once for all operations
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
	
	if (FJsonSerializer::Deserialize(Reader, RootObject) && RootObject.IsValid())
	{
		// 1. Extract Logging Info
		RootObject->TryGetStringField(TEXT("model"), LogModel);
		
		const TArray<TSharedPtr<FJsonValue>>* MessagesArrayPtr;
		if (RootObject->TryGetArrayField(TEXT("messages"), MessagesArrayPtr))
		{
			// Find the last user message for logging
			for (int32 i = MessagesArrayPtr->Num() - 1; i >= 0; --i)
			{
				TSharedPtr<FJsonObject> MessageObj = (*MessagesArrayPtr)[i]->AsObject();
				FString Role;
				if (MessageObj.IsValid() && MessageObj->TryGetStringField(TEXT("role"), Role) && Role == TEXT("user"))
				{
					MessageObj->TryGetStringField(TEXT("content"), LogPrompt);
					break;
				}
			}

			// 2. Inject LastResponses (if present)
			if (LastResponses.Num() > 0)
			{
				TArray<TSharedPtr<FJsonValue>> MessagesArray = *MessagesArrayPtr;
				int32 InsertIndex = MessagesArray.Num() > 1 ? MessagesArray.Num() - 1 : 1;

				for (const FString& Response : LastResponses)
				{
					TSharedPtr<FJsonObject> AssistantMessageObj = MakeShareable(new FJsonObject);
					AssistantMessageObj->SetStringField("role", "assistant");
					AssistantMessageObj->SetStringField("content", Response);
					
					if (InsertIndex < MessagesArray.Num())
					{
						MessagesArray.Insert(MakeShareable(new FJsonValueObject(AssistantMessageObj)), InsertIndex);
						InsertIndex++;
					}
					else
					{
						MessagesArray.Add(MakeShareable(new FJsonValueObject(AssistantMessageObj)));
					}
				}
				RootObject->SetArrayField("messages", MessagesArray);
			}
		}

		// 3. Inject 'input' field if missing (Fix for 400 error)
		// Some endpoints (like embeddings or specific completions) require 'input'.
		// We use the LogPrompt (last user message) as the input.
		if (!RootObject->HasField(TEXT("input")) && !LogPrompt.IsEmpty())
		{
			RootObject->SetStringField(TEXT("input"), LogPrompt);
		}

		// Serialize back to String
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonContent);
		FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
	}

	UE_LOG(LogTemp, Log, TEXT("Request-URL: %s"), *ServerURL);
	UE_LOG(LogTemp, Log, TEXT("Request-JSON: %s"), *JsonContent);

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
	double EndTime = FPlatformTime::Seconds();
	double Duration = EndTime - StartTime;
	
	FString ResponseContent = TEXT("");
	int32 ResponseCode = 0;

	if (bWasSuccessful && Response.IsValid())
	{
		ResponseContent = Response->GetContentAsString();
		ResponseCode = Response->GetResponseCode();

		if (ResponseCode >= 200 && ResponseCode < 300)
		{
			OnSuccess.Broadcast(ResponseContent, TEXT(""), true);
		}
		else
		{
			OnFailure.Broadcast(ResponseContent, FString::Printf(TEXT("HTTP Error: %d"), ResponseCode), false);
		}
	}
	else
	{
		OnFailure.Broadcast(TEXT(""), TEXT("Connection failed"), false);
	}

	// Log to file
	if (!LogFilePath.IsEmpty())
	{
		// Format: Time | Model | Status | Duration | URL | Prompt | Response
		FString LogEntry = FString::Printf(TEXT("[%s] Model: %s | Status: %d | Duration: %.4f s | URL: %s\nPrompt: %s\nResponse: %s\n--------------------------------------------------\n"), 
			*FDateTime::Now().ToString(), 
			*LogModel, 
			ResponseCode, 
			Duration, 
			*ServerURL,
			*LogPrompt, 
			*ResponseContent);

		FFileHelper::SaveStringToFile(LogEntry, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}
	
	SetReadyToDestroy();
}
