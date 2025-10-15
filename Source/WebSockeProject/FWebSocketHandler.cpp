#include "FWebSocketHandler.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"
#include "Misc/SecureHash.h"
#include "Misc/Base64.h"
#include "Math/UnrealMathUtility.h"
#include "Modules/ModuleManager.h"

FWebSocketHandler::FWebSocketHandler()
{
	// Example initialization
	UniqueId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
}

void FWebSocketHandler::Connect(const FString& Url)
{
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	Socket = FWebSocketsModule::Get().CreateWebSocket(Url);

	if (!Socket.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Err: Failed to create WebSocket: %s"), *Url);
		OnError.Broadcast(TEXT("Failed to create WebSocket"));
		return;
	}

	Socket->OnConnected().AddLambda([this]()
	{
		UE_LOG(LogTemp, Log, TEXT("WebSocket connected"));
		OnConnected.Broadcast();
	});

	Socket->OnConnectionError().AddLambda([this](const FString& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("WebSocket error: %s"), *Error);
		OnError.Broadcast(Error);
	});

	Socket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		UE_LOG(LogTemp, Warning, TEXT("WebSocket closed (Code: %d): %s"), StatusCode, *Reason);
		OnClosed.Broadcast();

		// Prevent dangling socket references
		Socket = nullptr;
	});

	Socket->OnMessage().AddLambda([this](const FString& Message)
	{
		UE_LOG(LogTemp, Log, TEXT("WebSocket message: %s"), *Message);
		OnMessage.Broadcast(Message);

		// Try to parse JSON
		TSharedPtr<FJsonObject> JsonObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			const FString Event = JsonObject->GetStringField(TEXT("event"));

			if (Event == TEXT("login"))
			{
				const FString Status = JsonObject->GetStringField(TEXT("status"));
				if (Status == TEXT("success"))
				{
					FString Token;
					if (JsonObject->TryGetStringField(TEXT("token"), Token))
					{
						UE_LOG(LogTemp, Log, TEXT("✅ Login success! Token: %s"), *Token);

						// Option 1: store locally
						LastReceivedToken = Token;

						// Option 2: broadcast token event (if Blueprint needs it)
						OnTokenReceived.Broadcast(Token);  // Fire the C++ delegate
						OnLoginStatusChanged.Broadcast(true);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Login failed."));
					OnLoginStatusChanged.Broadcast(false);
				}
			} else if (Event == TEXT("client_id"))
			{
				const FString Payload = JsonObject->GetStringField(TEXT("payload"));
				ClientId = Payload;
				OnClientIdReceived.Broadcast(ClientId);
			}
		}
	});

	Socket->Connect();
}

void FWebSocketHandler::SendMessage(const FString& Message) const
{
	if (Socket.IsValid() && Socket->IsConnected())
	{
		Socket->Send(Message);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot send message: WebSocket not connected"));
		OnError.Broadcast(TEXT("WebSocket not connected"));
	}
}

void FWebSocketHandler::Close()
{
	if (!Socket.IsValid())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Closing WebSocket connection gracefully..."));

	
	// Bind a temporary close handler to confirm shutdown
	bool bAlreadyClosed = false;
	Socket->OnClosed().AddLambda([this, &bAlreadyClosed](int32 Code, const FString& Reason, bool bWasClean)
	{
		UE_LOG(LogTemp, Log, TEXT("WebSocket closed cleanly (Code %d): %s"), Code, *Reason);
		bAlreadyClosed = true;
		Socket = nullptr;
	});

	// Trigger graceful shutdown
	Socket->Close(1000, TEXT("Client disconnecting"));

	// Wait a short moment (optional, for PIE or blocking cleanup)
	const double StartTime = FPlatformTime::Seconds();
	while (!bAlreadyClosed && (FPlatformTime::Seconds() - StartTime) < 0.25)
	{
		FPlatformProcess::Sleep(0.01f);
	}

	if (Socket.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WebSocket force-cleaning after timeout"));
		Socket->OnClosed().Clear();
		Socket->OnMessage().Clear();
		Socket->OnConnectionError().Clear();
		Socket = nullptr;
	}
}

// Constructs the WSURL by a Server Host string
FString FWebSocketHandler::ConstructWSURL(const FString& ServerHost, const int32& ServerPort, const FString& Endpoint, bool bSecure)
{
	FString Protocol = bSecure ? TEXT("wss://") : TEXT("ws://");

	FString NormalizedHost = ServerHost;
	if (NormalizedHost.StartsWith(TEXT("ws://")) || NormalizedHost.StartsWith(TEXT("wss://")))
	{
		int32 SchemeEndIndex = NormalizedHost.Find(TEXT("://"));
		if (SchemeEndIndex != INDEX_NONE)
		{
			NormalizedHost = NormalizedHost.RightChop(SchemeEndIndex + 3);
		}
	}

	FString NormalizedEndpoint = Endpoint.StartsWith(TEXT("/")) ? Endpoint : TEXT("/") + Endpoint;

	FString Url = FString::Printf(TEXT("%s%s:%d%s"), *Protocol, *NormalizedHost, ServerPort, *NormalizedEndpoint);

	return Url;
}


void FWebSocketHandler::RegisterUser(const FString& Email, const FString& Password) const
{
    TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
    Json->SetStringField(TEXT("event"), TEXT("register"));
    Json->SetStringField(TEXT("email"), Email);
    Json->SetStringField(TEXT("password"), Password);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

    SendMessage(OutputString);
}

void FWebSocketHandler::LoginUser(const FString& Email, const FString& Password) const
{
    TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
    Json->SetStringField(TEXT("event"), TEXT("login"));
    Json->SetStringField(TEXT("email"), Email);
    Json->SetStringField(TEXT("password"), Password);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

    SendMessage(OutputString);
}

void FWebSocketHandler::LoginWithJWT(const FString& Token) const
{
    TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
    Json->SetStringField(TEXT("event"), TEXT("jwt_login"));
    Json->SetStringField(TEXT("token"), Token);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

    SendMessage(OutputString);
}

bool FWebSocketHandler::VerifyJWT(const FString& Token, const FString& Secret)
{
    TArray<FString> Segments;
    Token.ParseIntoArray(Segments, TEXT("."));
    if (Segments.Num() != 3) return false;

    FString HeaderAndPayload = Segments[0] + TEXT(".") + Segments[1];
    FSHAHash Hash;
    FSHA1::HashBuffer(TCHAR_TO_ANSI(*HeaderAndPayload), HeaderAndPayload.Len(), Hash.Hash);

    FString LocalSignature = FBase64::Encode(Hash.Hash, sizeof(Hash.Hash));
    return LocalSignature.Equals(Segments[2], ESearchCase::IgnoreCase);
}

FString FWebSocketHandler::GenerateSalt(int32 Length)
{
    FString Salt;
    const FString Charset = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    for (int32 i = 0; i < Length; i++)
    {
        int32 Index = FMath::RandRange(0, Charset.Len() - 1);
        Salt.AppendChar(Charset[Index]);
    }

    return Salt;
}

FString FWebSocketHandler::HashPassword(const FString& Password, const FString& Salt)
{
    const FString Combined = Password + Salt;

    FTCHARToUTF8 Utf8Converter(*Combined);
    const uint8* Data = reinterpret_cast<const uint8*>(Utf8Converter.Get());
    const int32 NumBytes = Utf8Converter.Length();

    uint8 Hash[20] = {0};
    FSHA1::HashBuffer(Data, NumBytes, Hash);

    return FBase64::Encode(Hash, sizeof(Hash));
}

FWebSocketHandler::~FWebSocketHandler()
{
	if (Socket.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("FWebSocketHandler::~FWebSocketHandler(): closing socket on destruction"));
		Close();
	}
}
