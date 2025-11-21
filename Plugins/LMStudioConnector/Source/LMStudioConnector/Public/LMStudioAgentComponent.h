#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LMStudioAgentComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LMSTUDIOCONNECTOR_API ULMStudioAgentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULMStudioAgentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Parses a JSON string to extract an agent action and executes it.
	 * Expected JSON format: {"action": "response", "value": "PLAYER_MOVE_FORWARD"}
	 * 
	 * @param JsonString The JSON string containing the action.
	 */
	UFUNCTION(BlueprintCallable, Category = "LMStudio|Agent")
	void ProcessAgentAction(const FString& JsonString);

private:
	void ExecuteAction(const FString& ActionValue);
};
