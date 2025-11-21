#include "LMStudioAgentComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"

// Sets default values for this component's properties
ULMStudioAgentComponent::ULMStudioAgentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void ULMStudioAgentComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void ULMStudioAgentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULMStudioAgentComponent::ProcessAgentAction(const FString& JsonString)
{
	if (JsonString.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMStudioAgentComponent::ProcessAgentAction: Empty JSON string provided."));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		FString ActionValue;
		if (JsonObject->TryGetStringField(TEXT("value"), ActionValue))
		{
			ExecuteAction(ActionValue);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ULMStudioAgentComponent::ProcessAgentAction: 'value' field not found in JSON."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ULMStudioAgentComponent::ProcessAgentAction: Failed to parse JSON string: %s"), *JsonString);
	}
}

void ULMStudioAgentComponent::ExecuteAction(const FString& ActionValue)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMStudioAgentComponent::ExecuteAction: Owner is not a Pawn. Cannot execute movement actions."));
		return;
	}

	if (ActionValue == TEXT("PLAYER_MOVE_FORWARD"))
	{
		OwnerPawn->AddMovementInput(OwnerPawn->GetActorForwardVector(), 1.0f);
	}
	else if (ActionValue == TEXT("PLAYER_MOVE_BACKWARD"))
	{
		OwnerPawn->AddMovementInput(OwnerPawn->GetActorForwardVector(), -1.0f);
	}
	else if (ActionValue == TEXT("PLAYER_MOVE_LEFT"))
	{
		OwnerPawn->AddMovementInput(OwnerPawn->GetActorRightVector(), -1.0f);
	}
	else if (ActionValue == TEXT("PLAYER_MOVE_RIGHT"))
	{
		OwnerPawn->AddMovementInput(OwnerPawn->GetActorRightVector(), 1.0f);
	}
	else if (ActionValue == TEXT("PLAYER_MOVE_JUMP"))
	{
		ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn);
		if (OwnerCharacter)
		{
			OwnerCharacter->Jump();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ULMStudioAgentComponent::ExecuteAction: Owner is not a Character. Cannot execute Jump action."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ULMStudioAgentComponent::ExecuteAction: Unknown action value: %s"), *ActionValue);
	}
}
