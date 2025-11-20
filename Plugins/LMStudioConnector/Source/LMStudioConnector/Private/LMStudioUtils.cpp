#include "LMStudioUtils.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"

FString ULMStudioUtils::CreateLMStudioRequestJSON(const FString& Model, const FString& SystemPrompt, const FString& UserMessage, float Temperature)
{
	// Create the root object
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
	RootObject->SetStringField("model", Model);
	RootObject->SetNumberField("temperature", Temperature);
	RootObject->SetNumberField("max_tokens", -1);
	RootObject->SetBoolField("stream", false);

	// Create the messages array
	TArray<TSharedPtr<FJsonValue>> MessagesArray;

	// System Message
	TSharedPtr<FJsonObject> SystemMessageObj = MakeShareable(new FJsonObject);
	SystemMessageObj->SetStringField("role", "system");
	SystemMessageObj->SetStringField("content", SystemPrompt);
	MessagesArray.Add(MakeShareable(new FJsonValueObject(SystemMessageObj)));

	// User Message
	TSharedPtr<FJsonObject> UserMessageObj = MakeShareable(new FJsonObject);
	UserMessageObj->SetStringField("role", "user");
	UserMessageObj->SetStringField("content", UserMessage);
	MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMessageObj)));

	RootObject->SetArrayField("messages", MessagesArray);

	// Serialize to String
	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}
