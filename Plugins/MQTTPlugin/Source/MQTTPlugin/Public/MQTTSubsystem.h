#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MQTTClientHelper.h"
#include "MQTTSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMQTTMessageReceivedEvent, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMQTTConnectedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMQTTDisconnectedEvent);

UCLASS(BlueprintType, Blueprintable)
class MQTTPLUGIN_API UMQTTSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="MQTT", meta=(WorldContext="WorldContextObject"))
	static UMQTTSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Connect with optional Home Assistant credentials */
	UFUNCTION(BlueprintCallable, Category="MQTT|Connection")
	bool Connect(const FString& Broker, int32 Port = 1883, const FString& ClientId = TEXT("UnrealClient"),
				 const FString& Username = TEXT(""), const FString& Password = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="MQTT|Connection")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category="MQTT|Topics")
	bool Subscribe(const FString& Topic, int32 QoS = 0);

	UFUNCTION(BlueprintCallable, Category="MQTT|Topics")
	bool Publish(const FString& Topic, const FString& Message, int32 QoS = 0);

	/** Helper to publish a Home Assistant Discovery message */
	UFUNCTION(BlueprintCallable, Category="MQTT|HomeAssistant")
	bool PublishDiscovery(const FString& ObjectId, const FString& FriendlyName, const FString& ComponentType, const FString& Unit = TEXT(""));

	UPROPERTY(BlueprintAssignable, Category="MQTT|Events")
	FOnMQTTConnectedEvent OnConnected;

	UPROPERTY(BlueprintAssignable, Category="MQTT|Events")
	FOnMQTTDisconnectedEvent OnDisconnected;

	UPROPERTY(BlueprintAssignable, Category="MQTT|Events")
	FOnMQTTMessageReceivedEvent OnMessageReceived;

private:
	UPROPERTY()
	UMQTTClientHelper* MQTTClient;

	UFUNCTION()
	void HandleMQTTMessage(const FString& Message);

	FString CurrentUsername;
	FString CurrentPassword;
};
