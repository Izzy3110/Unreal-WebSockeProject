#pragma once
#include "CoreMinimal.h"
#include "IWebSocket.h"

DECLARE_MULTICAST_DELEGATE(FWebSocketConnected);
DECLARE_MULTICAST_DELEGATE_OneParam(FWebSocketError, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FWebSocketMessage, const FString&);
DECLARE_MULTICAST_DELEGATE(FWebSocketClosed);
DECLARE_MULTICAST_DELEGATE_OneParam(FWebSocketTokenReceived, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FWebSocketClientIdReceived, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FWebSocketLoginStatusChanged, bool);

class FWebSocketHandler
{
public:
	FWebSocketHandler();
	~FWebSocketHandler();

	void Connect(const FString& Url);
	void SendMessage(const FString& Message) const;
	void Close();

	FString GetUniqueId() const { return UniqueId; }

	// User functions
	void RegisterUser(const FString& Email, const FString& Password) const;
	void LoginUser(const FString& Email, const FString& Password) const;
	void LoginWithJWT(const FString& Token) const;
	static bool VerifyJWT(const FString& Token, const FString& Secret);
	bool IsConnected() const { return Socket.IsValid() && Socket->IsConnected(); }
	static FString ConstructWSURL(const FString& ServerHost, const int32& ServerPort, const FString& Endpoint, bool bSecure = false);

private:
	static FString GenerateSalt(int32 Length = 16);
	static FString HashPassword(const FString& Password, const FString& Salt);

private:
	TSharedPtr<IWebSocket> Socket;
	FString UniqueId;

public:
	FWebSocketConnected OnConnected;
	FWebSocketError OnError;
	FWebSocketMessage OnMessage;
	FWebSocketClosed OnClosed;
	FWebSocketTokenReceived OnTokenReceived;
	FWebSocketClientIdReceived OnClientIdReceived;
	FWebSocketLoginStatusChanged OnLoginStatusChanged;
	FString LastReceivedToken; // optional storage
	FString ClientId;
};
