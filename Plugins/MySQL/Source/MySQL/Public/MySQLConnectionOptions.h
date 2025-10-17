// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MySQLConnectionOptions.generated.h"

UENUM(BlueprintType)
enum class EMySQLOptions : uint8
{
    OPT_CONNECT_TIMEOUT UMETA(DisplayName = "Connect Timeout"),
    OPT_COMPRESS UMETA(DisplayName = "Compress"),
    OPT_NAMED_PIPE UMETA(DisplayName = "Named Pipe"),
    INIT_COMMAND UMETA(DisplayName = "Init Command"),
    READ_DEFAULT_FILE UMETA(DisplayName = "Read Default File"),
    READ_DEFAULT_GROUP UMETA(DisplayName = "Read Default Group"),
    SET_CHARSET_DIR UMETA(DisplayName = "Set Charset Dir"),
    SET_CHARSET_NAME UMETA(DisplayName = "Set Charset Name"),
    OPT_LOCAL_INFILE UMETA(DisplayName = "Local Infile"),
    OPT_PROTOCOL UMETA(DisplayName = "Protocol"),
    SHARED_MEMORY_BASE_NAME UMETA(DisplayName = "Shared Memory Base Name"),
    OPT_READ_TIMEOUT UMETA(DisplayName = "Read Timeout"),
    OPT_WRITE_TIMEOUT UMETA(DisplayName = "Write Timeout"),
    OPT_USE_RESULT UMETA(DisplayName = "Use Result"),
    OPT_USE_REMOTE_CONNECTION UMETA(DisplayName = "Use Remote Connection"),
    OPT_USE_EMBEDDED_CONNECTION UMETA(DisplayName = "Use Embedded Connection"),
    OPT_GUESS_CONNECTION UMETA(DisplayName = "Guess Connection"),
    SET_CLIENT_IP UMETA(DisplayName = "Set Client IP"),
    SECURE_AUTH UMETA(DisplayName = "Secure Auth"),
    REPORT_DATA_TRUNCATION UMETA(DisplayName = "Report Data Truncation"),
    OPT_RECONNECT UMETA(DisplayName = "Reconnect"),
    OPT_SSL_VERIFY_SERVER_CERT UMETA(DisplayName = "SSL Verify Server Cert"),
    DEFAULT_AUTH UMETA(DisplayName = "Default Auth"),
    OPT_BIND UMETA(DisplayName = "Bind"),
    OPT_SSL_KEY UMETA(DisplayName = "SSL Key"),
    OPT_SSL_CERT UMETA(DisplayName = "SSL Cert"),
    OPT_SSL_CA UMETA(DisplayName = "SSL CA"),
    OPT_SSL_CAPATH UMETA(DisplayName = "SSL CA Path"),
    OPT_SSL_CIPHER UMETA(DisplayName = "SSL Cipher"),
    OPT_SSL_CRL UMETA(DisplayName = "SSL CRL"),
    OPT_SSL_CRLPATH UMETA(DisplayName = "SSL CRL Path"),
    OPT_CONNECT_ATTR_RESET UMETA(DisplayName = "Connect Attr Reset"),
    OPT_CONNECT_ATTR_ADD UMETA(DisplayName = "Connect Attr Add"),
    OPT_CONNECT_ATTR_DELETE UMETA(DisplayName = "Connect Attr Delete"),
    SERVER_PUBLIC_KEY UMETA(DisplayName = "Server Public Key"),
    ENABLE_CLEARTEXT_PLUGIN UMETA(DisplayName = "Enable Cleartext Plugin"),
    OPT_CAN_HANDLE_EXPIRED_PASSWORDS UMETA(DisplayName = "Can Handle Expired Passwords"),
    OPT_SSL_ENFORCE UMETA(DisplayName = "SSL Enforce"),
    OPT_MAX_ALLOWED_PACKET UMETA(DisplayName = "Max Allowed Packet"),
    OPT_NET_BUFFER_LENGTH UMETA(DisplayName = "Net Buffer Length"),
    OPT_TLS_VERSION UMETA(DisplayName = "TLS Version"),
    PROGRESS_CALLBACK UMETA(DisplayName = "Progress Callback"),
    OPT_NONBLOCK UMETA(DisplayName = "NonBlock"),
    DATABASE_DRIVER UMETA(DisplayName = "Database Driver"),
    OPT_TLS_PASSPHRASE UMETA(DisplayName = "TLS Passphrase"),
    OPT_TLS_CIPHER_STRENGTH UMETA(DisplayName = "TLS Cipher Strength"),
    OPT_TLS_PEER_FP UMETA(DisplayName = "TLS Peer FP"),
    OPT_TLS_PEER_FP_LIST UMETA(DisplayName = "TLS Peer FP List"),
    OPT_CONNECTION_READ_ONLY UMETA(DisplayName = "Connection Read Only"),
    OPT_CONNECT_ATTRS UMETA(DisplayName = "Connect Attrs"),
    OPT_USERDATA UMETA(DisplayName = "Userdata"),
    OPT_CONNECTION_HANDLER UMETA(DisplayName = "Connection Handler"),
    OPT_FOUND_ROWS UMETA(DisplayName = "Found Rows"),
    OPT_MULTI_RESULTS UMETA(DisplayName = "Multi Results"),
    OPT_MULTI_STATEMENTS UMETA(DisplayName = "Multi Statements"),
    OPT_INTERACTIVE UMETA(DisplayName = "Interactive"),
    OPT_PROXY_HEADER UMETA(DisplayName = "Proxy Header"),
    OPT_IO_WAIT UMETA(DisplayName = "IO Wait"),
    OPT_SKIP_READ_RESPONSE UMETA(DisplayName = "Skip Read Response"),
    OPT_RESTRICTED_AUTH UMETA(DisplayName = "Restricted Auth"),
    OPT_RPL_REGISTER_REPLICA UMETA(DisplayName = "RPL Register Replica"),
    OPT_STATUS_CALLBACK UMETA(DisplayName = "Status Callback"),
    OPT_SERVER_PLUGINS UMETA(DisplayName = "Server Plugins")
};

USTRUCT(BlueprintType)
struct FMySQLOptionPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySQL Options")
	EMySQLOptions Option;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySQL Options")
	FString Value;
};
/**
 * 
 */
UCLASS()
class MYSQL_API UMySQLConnectionOptions : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MySQL Options")
	TArray<FMySQLOptionPair> ConnectionOptions;

};
