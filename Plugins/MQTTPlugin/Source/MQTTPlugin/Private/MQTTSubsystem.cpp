#include "MQTTSubsystem.h"
#include "MQTTClientHelper.h"

UMQTTSubsystem* UMQTTSubsystem::Get(const UObject* WorldContextObject)
{
	if (const UGameInstance* GI = WorldContextObject ? WorldContextObject->GetWorld()->GetGameInstance() : nullptr)
	{
		return GI->GetSubsystem<UMQTTSubsystem>();
	}
	return nullptr;
}

void UMQTTSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MQTTClient = NewObject<UMQTTClientHelper>(this);
	if (MQTTClient)
	{
		MQTTClient->AddToRoot();
		MQTTClient->OnMessage.AddDynamic(this, &UMQTTSubsystem::HandleMQTTMessage);
	}
}

void UMQTTSubsystem::Deinitialize()
{
	if (MQTTClient)
	{
		MQTTClient->Disconnect();
		MQTTClient->RemoveFromRoot();
		MQTTClient = nullptr;
	}

	Super::Deinitialize();
}

bool UMQTTSubsystem::Connect(const FString& Broker, int32 Port, const FString& ClientId,
                             const FString& Username, const FString& Password)
{
	if (!MQTTClient)
		return false;

	CurrentUsername = Username;
	CurrentPassword = Password;

	// You can modify UMQTTClientHelper::Connect to accept username/password
	const bool bResult = MQTTClient->Connect(Broker, Port, ClientId);

	if (bResult)
		OnConnected.Broadcast();
	else
		UE_LOG(LogTemp, Error, TEXT("MQTTSubsystem: Failed to connect to broker %s:%d"), *Broker, Port);

	return bResult;
}

void UMQTTSubsystem::Disconnect()
{
	if (!MQTTClient)
		return;

	MQTTClient->Disconnect();
	OnDisconnected.Broadcast();
}

bool UMQTTSubsystem::Subscribe(const FString& Topic, int32 QoS)
{
	if (!MQTTClient)
		return false;
	return MQTTClient->Subscribe(Topic, QoS);
}

bool UMQTTSubsystem::Publish(const FString& Topic, const FString& Message, int32 QoS)
{
	if (!MQTTClient)
		return false;
	return MQTTClient->Publish(Topic, Message, QoS);
}

bool UMQTTSubsystem::PublishDiscovery(const FString& ObjectId, const FString& FriendlyName,
                                      const FString& ComponentType, const FString& Unit)
{
	if (!MQTTClient)
		return false;

	// Example discovery topic for Home Assistant sensor
	const FString Topic = FString::Printf(TEXT("homeassistant/%s/%s/config"), *ComponentType, *ObjectId);

	const FString Payload = FString::Printf(
		TEXT("{\"name\": \"%s\", \"state_topic\": \"homeassistant/%s/%s/state\", \"unit_of_measurement\": \"%s\"}"),
		*FriendlyName, *ComponentType, *ObjectId, *Unit);

	return Publish(Topic, Payload, 0);
}

void UMQTTSubsystem::HandleMQTTMessage(const FString& Message)
{
	OnMessageReceived.Broadcast(Message);
}
