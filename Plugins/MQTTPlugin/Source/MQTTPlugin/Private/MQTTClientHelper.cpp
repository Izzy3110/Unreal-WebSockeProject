#include "MQTTClientHelper.h"
#include "Async/Async.h"
#include "MQTTAsync.h"

void UMQTTClientHelper::Disconnect()
{
	if (!Client)
		return;

	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	MQTTAsync_disconnect(Client, &disc_opts);
	MQTTAsync_destroy(reinterpret_cast<MQTTAsync*>(&Client));
	Client = nullptr;
	bConnected = false;
}

bool UMQTTClientHelper::Subscribe(const FString& Topic, int32 QoS)
{
	if (!bConnected || !Client)
		return false;

	int rc = MQTTAsync_subscribe(Client, TCHAR_TO_UTF8(*Topic), QoS, nullptr);
	return rc == MQTTASYNC_SUCCESS;
}

bool UMQTTClientHelper::Publish(const FString& Topic, const FString& Message, int32 QoS)
{
	if (!bConnected || !Client)
		return false;

	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	FTCHARToUTF8 MsgUtf8(*Message);
	pubmsg.payload = (void*)MsgUtf8.Get();
	pubmsg.payloadlen = MsgUtf8.Length();
	pubmsg.qos = QoS;
	pubmsg.retained = 0;

	int rc = MQTTAsync_sendMessage(Client, TCHAR_TO_UTF8(*Topic), &pubmsg, &opts);
	return rc == MQTTASYNC_SUCCESS;
}

void UMQTTClientHelper::OnConnectionLost(void* context, char* cause)
{
	if (UMQTTClientHelper* Self = static_cast<UMQTTClientHelper*>(context))
	{
		Self->bConnected = false;
		UE_LOG(LogTemp, Warning, TEXT("MQTT connection lost: %hs"), cause ? cause : "unknown");
	}
}

int UMQTTClientHelper::OnMessageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
	if (UMQTTClientHelper* Self = static_cast<UMQTTClientHelper*>(context))
	{
		const FString Payload = UTF8_TO_TCHAR(static_cast<char*>(message->payload));

		AsyncTask(ENamedThreads::GameThread, [Self, Payload]()
		{
			Self->OnMessage.Broadcast(Payload);
		});
	}

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;
}

bool UMQTTClientHelper::Connect(const FString& Broker, int32 Port, const FString& ClientId)
{
	BrokerAddress = Broker;
	BrokerPort = Port;

	FString Url = FString::Printf(TEXT("tcp://%s:%d"), *Broker, Port);

	if (MQTTAsync_create(reinterpret_cast<MQTTAsync*>(&Client),
		TCHAR_TO_UTF8(*Url),
		TCHAR_TO_UTF8(*ClientId),
		MQTTCLIENT_PERSISTENCE_NONE, nullptr) != MQTTASYNC_SUCCESS)
	{
		UE_LOG(LogTemp, Error, TEXT("MQTTAsync_create failed"));
		return false;
	}

	// ✅ Explicit function pointer cast to avoid Unreal's UHT confusion
	MQTTAsync_setCallbacks(Client, this,
		(MQTTAsync_connectionLost*)UMQTTClientHelper::OnConnectionLost,
		(MQTTAsync_messageArrived*)UMQTTClientHelper::OnMessageArrived,
		nullptr);

	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

	int rc = MQTTAsync_connect(Client, &conn_opts);
	bConnected = (rc == MQTTASYNC_SUCCESS);

	if (bConnected)
	{
		UE_LOG(LogTemp, Log, TEXT("MQTT connected to %s:%d"), *Broker, Port);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MQTT connection failed, rc=%d"), rc);
	}

	return bConnected;
}
