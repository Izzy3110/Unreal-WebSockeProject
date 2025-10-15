#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PostgresClient.generated.h"

// Match libpq's typedef exactly to avoid redefinition clashes.
// (libpq does: typedef struct pg_conn PGconn;)
struct pg_conn;
typedef struct pg_conn PGconn;

USTRUCT(BlueprintType)
struct FPostgresQueryResultRow
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) TMap<FString, FString> Values;
};

USTRUCT(BlueprintType)
struct FPostgresQueryResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bSuccess = false;
	UPROPERTY(BlueprintReadOnly) FString Error;
	UPROPERTY(BlueprintReadOnly) TArray<FString> Columns;
	UPROPERTY(BlueprintReadOnly) TArray<FPostgresQueryResultRow> Rows;
	UPROPERTY(BlueprintReadOnly) int32 RowsAffected = 0;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FPostgresQueryResultDelegate, const FPostgresQueryResult&, Result);

/**
 * Minimal libpq client for UE. Use Exec for blocking queries (not recommended on game thread)
 * and ExecAsync for threaded queries that marshal results back to the game thread.
 *
 * SQL must use $1, $2, ... parameters when passing Params.
 */
UCLASS(BlueprintType, Blueprintable)
class POSTGRES_API UPostgresClient : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Postgres")
	void SetConnectionString(const FString& InConnStr);

	UFUNCTION(BlueprintCallable, Category="Postgres")
	bool Connect();

	UFUNCTION(BlueprintCallable, Category="Postgres")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category="Postgres")
	bool IsConnected() const;

	/** Blocking query (no parameters). Do NOT call on the game thread for long queries. */
	UFUNCTION(BlueprintCallable, Category="Postgres")
	FPostgresQueryResult Exec(const FString& Sql);

	/** Blocking, parameterized. Use $1, $2... in Sql and fill Params in the same order. */
	UFUNCTION(BlueprintCallable, Category="Postgres")
	FPostgresQueryResult ExecParams(const FString& SqlDollarNumbered, const TArray<FString>& Params);

	/** Async, parameterized. Runs on the thread pool and returns to the game thread. */
	UFUNCTION(BlueprintCallable, Category="Postgres", meta=(DisplayName="Exec Async"))
	void ExecAsync(const FString& SqlDollarNumbered, const TArray<FString>& Params, const FPostgresQueryResultDelegate& OnCompleted);

protected:
	virtual void BeginDestroy() override;

private:
	FPostgresQueryResult ExecInternal(const FString& Sql, const TArray<FString>* ParamsOpt);

	/** Connection handle and state */
	PGconn* Conn = nullptr;
	FString ConnStr;
	mutable FCriticalSection ConnMutex;
};
