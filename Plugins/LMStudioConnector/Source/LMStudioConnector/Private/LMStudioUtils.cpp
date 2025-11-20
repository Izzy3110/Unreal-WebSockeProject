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

FString ULMStudioUtils::ExtractOutputContentText(const FString& JsonResponse)
{
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonResponse);

	if (FJsonSerializer::Deserialize(Reader, RootObject) && RootObject.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* OutputArray;
		if (RootObject->TryGetArrayField("output", OutputArray) && OutputArray->Num() > 0)
		{
			TSharedPtr<FJsonObject> OutputObject = (*OutputArray)[0]->AsObject();
			if (OutputObject.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* ContentArray;
				if (OutputObject->TryGetArrayField("content", ContentArray) && ContentArray->Num() > 0)
				{
					TSharedPtr<FJsonObject> ContentObject = (*ContentArray)[0]->AsObject();
					if (ContentObject.IsValid())
					{
						FString Text;
						if (ContentObject->TryGetStringField("text", Text))
						{
							return Text;
						}
					}
				}
			}
		}
	}

	return FString();
}
