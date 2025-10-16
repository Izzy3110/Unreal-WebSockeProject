#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FWebSocketHandler.h"
#include "BlueprintWebSocketClient.generated.h"

// Delegates for Blueprint Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessage, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketError, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTokenReceivedBP, const FString&, Token);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginStatusChangedBP, bool, LoginStatus);
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class WEBSOCKEPROJECT_API ABlueprintWebSocketClient : public AActor
{
	GENERATED_BODY()


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	ABlueprintWebSocketClient();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket|Parameters", meta = (AllowPrivateAccess = "true"))
	FString ServerHost = TEXT("/echo");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket|Parameters", meta = (AllowPrivateAccess = "true"))
	int32 DefaultPort = 9090;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket|Parameters", meta = (AllowPrivateAccess = "true"))
	FString Path = TEXT("/echo");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket|Parameters|Auth", meta = (AllowPrivateAccess = "true"))
	FString JWT_Token = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket|Parameters|Auth", meta = (AllowPrivateAccess = "true"))
	FString JWT_Secret = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket|Parameters", meta = (AllowPrivateAccess = "true"))
	bool JWT_Valid;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocket|Parameters", meta = (AllowPrivateAccess = "true"))
	FString CurrentUrl;
	
	UFUNCTION(BlueprintCallable, Category = "WebSocket|Utilities")
	FString ConstructWebSocketURL(const FString& Host, const int32& ServerPort, const FString& Endpoint, bool bSecure);

	UFUNCTION(BlueprintCallable, Category = "WebSocket|Connection")
	void Connect(const FString& Url);

	UFUNCTION(BlueprintCallable, Category = "WebSocket|Auth")
	void RegisterUser(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "WebSocket|Auth")
	void LoginUser(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "WebSocket|Auth")
	void LoginWithJWT(const FString& Token);

	UFUNCTION(BlueprintCallable, Category = "WebSocket|Auth")
	bool VerifyJWT(const FString& Token, const FString& Secret);
	
	UFUNCTION(BlueprintCallable, Category = "WebSocket|Messaging")
	void SendMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "WebSocket|Connection")
	void Close();

	UFUNCTION(BlueprintPure, Category = "WebSocket|Connection")
	bool IsConnected() const;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketConnected OnConnected;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketMessage OnMessage;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketClosed OnClosed;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketError OnError;

	// Blueprint delegate
	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnTokenReceivedBP OnTokenReceived;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnLoginStatusChangedBP OnLoginStatusChanged;

private:
	TUniquePtr<FWebSocketHandler> WebSocketHandler;
};
