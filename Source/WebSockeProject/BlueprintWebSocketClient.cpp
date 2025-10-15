// Fill out your copyright notice in the Description page of Project Settings.
#include "BlueprintWebSocketClient.h"
#include "Modules/ModuleManager.h"          // FModuleManager
#include "Misc/SecureHash.h"                // FSHA1
#include "Misc/Base64.h"                    // FBase64
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Math/UnrealMathUtility.h"         // FMath (if not already included)

// Sets default values
ABlueprintWebSocketClient::ABlueprintWebSocketClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABlueprintWebSocketClient::BeginPlay()
{
	Super::BeginPlay();
}

void ABlueprintWebSocketClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (WebSocketHandler)
	{
		UE_LOG(LogTemp, Log, TEXT("ABlueprintWebSocketClient shutting down WebSocket connection..."));

		// Gracefully close WebSocket before destruction
		WebSocketHandler->Close();
		WebSocketHandler.Reset();
	}
	
	Super::EndPlay(EndPlayReason);
}


// Called every frame
void ABlueprintWebSocketClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Constructs the WSURL by a Server Host string
FString ABlueprintWebSocketClient::ConstructWSURL(const FString& Host, const int32& Port, const FString& Endpoint, bool bSecure)
{
	if (!WebSocketHandler)
	{
		WebSocketHandler = MakeUnique<FWebSocketHandler>();
	}
	if (WebSocketHandler)
	{
		return WebSocketHandler->ConstructWSURL(Host, Port, Endpoint);
	}
	return "";
}


// Connect to WebSocket server
void ABlueprintWebSocketClient::Connect(const FString& Url)
{
	WebSocketHandler = MakeUnique<FWebSocketHandler>();

	WebSocketHandler->OnTokenReceived.AddLambda([this](const FString& Token)
	{
		OnTokenReceived.Broadcast(Token);  // Forward to Blueprint
	});

	WebSocketHandler->OnConnected.AddLambda([this]()
	{
		OnConnected.Broadcast();
	});

	WebSocketHandler->OnMessage.AddLambda([this](const FString& Message)
	{
		OnMessage.Broadcast(Message);
	});

	WebSocketHandler->OnError.AddLambda([this](const FString& Error)
	{
		OnError.Broadcast(Error);
	});

	WebSocketHandler->OnClosed.AddLambda([this]()
	{
		OnClosed.Broadcast();
	});

	WebSocketHandler->Connect(Url);
}

void ABlueprintWebSocketClient::RegisterUser(const FString& Email, const FString& Password)
{
	if (WebSocketHandler)
	{
		WebSocketHandler->RegisterUser(Email, Password);	
	}
}

void ABlueprintWebSocketClient::LoginUser(const FString& Email, const FString& Password)
{
	if (WebSocketHandler)
	{
		WebSocketHandler->LoginUser(Email, Password);	
	}
}
// ... other functions ...

void ABlueprintWebSocketClient::LoginWithJWT(const FString& Token)
{
	if (WebSocketHandler)
	{
		WebSocketHandler->LoginWithJWT(Token);
	}
}

bool ABlueprintWebSocketClient::VerifyJWT(const FString& Token, const FString& Secret)
{
	if (WebSocketHandler)
	{
		return WebSocketHandler->VerifyJWT(Token, Secret);
	}
	return false;
}

// Send a message
void ABlueprintWebSocketClient::SendMessage(const FString& Message)
{
	if (WebSocketHandler)
	{
		WebSocketHandler->SendMessage(Message);
	}
}

// Close the WebSocket connection
void ABlueprintWebSocketClient::Close()
{
	if (WebSocketHandler)
	{
		WebSocketHandler->Close();
	}
}

// Close the WebSocket connection
bool ABlueprintWebSocketClient::IsConnected() const
{
	if (WebSocketHandler)
	{
		return WebSocketHandler->IsConnected();
	}
	return false;
}