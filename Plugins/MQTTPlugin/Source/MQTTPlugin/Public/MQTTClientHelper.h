#pragma once
#include <MQTTAsync.h>

#include "CoreMinimal.h"
#include "MQTTClientHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMQTTMessageReceived, const FString&, Payload);

UCLASS(BlueprintType)
class MQTTPLUGIN_API UMQTTClientHelper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="MQTT")
	FOnMQTTMessageReceived OnMessage;

	UFUNCTION(BlueprintCallable, Category="MQTT")
	bool Connect(const FString& Broker, int32 Port, const FString& ClientId);

	UFUNCTION(BlueprintCallable, Category="MQTT")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category="MQTT")
	bool Subscribe(const FString& Topic, int32 QoS = 0);

	UFUNCTION(BlueprintCallable, Category="MQTT")
	bool Publish(const FString& Topic, const FString& Message, int32 QoS = 0);

private:
	// ✅ Paho MQTTAsync callback signatures
	static void OnConnectionLost(void* Context, char* Cause);
	static int OnMessageArrived(void* Context, char* TopicName, int TopicLen, MQTTAsync_message* Message);

private:
	void* Client = nullptr;
	bool bConnected = false;
	FString BrokerAddress;
	int32 BrokerPort;
};
