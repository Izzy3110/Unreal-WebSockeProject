// Must be first for explicit PCH
#include "Postgres.h"

#include "PostgresClient.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ScopeLock.h"
#include "Containers/StringConv.h" // FTCHARToUTF8 / UTF8_TO_TCHAR

THIRD_PARTY_INCLUDES_START
#include "libpq-fe.h"
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY_STATIC(LogPostgres, Log, All);

// Unique helper name to avoid clashing with UE::MakeError
static FPostgresQueryResult MakePgError(const FString& Message)
{
	FPostgresQueryResult R;
	R.bSuccess = false;
	R.Error = Message;
	return R;
}

void UPostgresClient::SetConnectionString(const FString& InConnStr)
{
	ConnStr = InConnStr;
}

bool UPostgresClient::Connect()
{
#if PLATFORM_WINDOWS
	if (!Postgres_EnsureLibpqLoaded())
	{
		UE_LOG(LogPostgres, Error, TEXT("libpq preload failed. See earlier [Postgres] log for missing DLL(s)."));
		return false;
	}
#endif
	
	FScopeLock Lock(&ConnMutex);

	if (Conn && PQstatus(Conn) == CONNECTION_OK) return true;

	if (Conn) { PQfinish(Conn); Conn = nullptr;	}

	FTCHARToUTF8 ConnUtf8(*ConnStr);
	Conn = PQconnectdb(ConnUtf8.Get());
	if (!Conn || PQstatus(Conn) != CONNECTION_OK)
	{
		FString Err = Conn ? UTF8_TO_TCHAR(PQerrorMessage(Conn)) : TEXT("PQconnectdb returned null.");
		if (Conn) { PQfinish(Conn); Conn = nullptr; }
		UE_LOG(LogPostgres, Error, TEXT("Postgres connect failed: %s"), *Err);
		return false;
	}
	return true;
}

void UPostgresClient::Disconnect()
{
	FScopeLock Lock(&ConnMutex);
	if (Conn)
	{
		PQfinish(Conn);
		Conn = nullptr;
	}
}

bool UPostgresClient::IsConnected() const
{
	FScopeLock Lock(&ConnMutex);
	return Conn && PQstatus(Conn) == CONNECTION_OK;
}

FPostgresQueryResult UPostgresClient::Exec(const FString& Sql)
{
	return ExecInternal(Sql, nullptr);
}

FPostgresQueryResult UPostgresClient::ExecParams(const FString& Sql, const TArray<FString>& Params)
{
	return ExecInternal(Sql, &Params);
}

void UPostgresClient::ExecAsync(const FString& Sql, const TArray<FString>& Params, const FPostgresQueryResultDelegate& OnCompleted)
{
	// Copy everything we need
	TWeakObjectPtr<UPostgresClient> Self(this);
	const FString SqlCopy = Sql;
	const TArray<FString> ParamsCopy = Params;
	const FPostgresQueryResultDelegate Callback = OnCompleted;

	Async(EAsyncExecution::ThreadPool, [Self, SqlCopy, ParamsCopy, Callback]()
	{
		if (!Self.IsValid())
		{
			return;
		}

		FPostgresQueryResult Result = Self->ExecInternal(SqlCopy, &ParamsCopy);

		AsyncTask(ENamedThreads::GameThread, [Self, Result, Callback]()
		{
			if (Self.IsValid() && Callback.IsBound())
			{
				Callback.Execute(Result);
			}
		});
	});
}

FPostgresQueryResult UPostgresClient::ExecInternal(const FString& Sql, const TArray<FString>* ParamsOpt)
{
	// Ensure we have a live connection (connect will also log errors)
	if (!Connect())
	{
		return ::MakePgError(TEXT("Not connected to PostgreSQL."));
	}

	FScopeLock Lock(&ConnMutex);

	FTCHARToUTF8 SqlUtf8(*Sql);
	PGresult* PgRes = nullptr;

	if (ParamsOpt && ParamsOpt->Num() > 0)
	{
		const int32 N = ParamsOpt->Num();
		TArray<FTCHARToUTF8> ParamUtf8;   ParamUtf8.Reserve(N);
		TArray<const char*> Values;       Values.Reserve(N);
		TArray<int> Lengths;              Lengths.Init(0, N);
		TArray<int> Formats;              Formats.Init(0, N); // 0 = text

		for (const FString& P : *ParamsOpt)
		{
			ParamUtf8.Emplace(*P);
			Values.Add(ParamUtf8.Last().Get());   // lifetime tied to ParamUtf8 element
		}

		PgRes = PQexecParams(Conn,
							 SqlUtf8.Get(),
							 N,
							 nullptr,                // infer types
							 Values.GetData(),
							 Lengths.GetData(),
							 Formats.GetData(),
							 0);                    // 0 = text results
	}
	else
	{
		PgRes = PQexec(Conn, SqlUtf8.Get());
	}

	if (!PgRes)
	{
		return ::MakePgError(TEXT("PQexec returned null."));
	}

	ExecStatusType Status = PQresultStatus(PgRes);

	FPostgresQueryResult Out;

	if (Status == PGRES_TUPLES_OK)
	{
		const int32 Cols = PQnfields(PgRes);
		const int32 Rows = PQntuples(PgRes);

		Out.Columns.Reserve(Cols);
		for (int32 c = 0; c < Cols; ++c)
		{
			Out.Columns.Add(UTF8_TO_TCHAR(PQfname(PgRes, c)));
		}

		Out.Rows.Reserve(Rows);
		for (int32 r = 0; r < Rows; ++r)
		{
			FPostgresQueryResultRow Row;
			for (int32 c = 0; c < Cols; ++c)
			{
				const FString& Key = Out.Columns[c];
				FString Val;
				if (!PQgetisnull(PgRes, r, c))
				{
					Val = UTF8_TO_TCHAR(PQgetvalue(PgRes, r, c));
				}
				Row.Values.Add(Key, MoveTemp(Val));
			}
			Out.Rows.Add(MoveTemp(Row));
		}

		Out.bSuccess = true;
	}
	else if (Status == PGRES_COMMAND_OK)
	{
		const char* Affected = PQcmdTuples(PgRes); // may be ""
		Out.RowsAffected = (Affected && *Affected) ? FCStringAnsi::Atoi(Affected) : 0;
		Out.bSuccess = true;
	}
	else
	{
		Out.bSuccess = false;
		Out.Error = UTF8_TO_TCHAR(PQresultErrorMessage(PgRes));
	}

	PQclear(PgRes);
	return Out;
}

void UPostgresClient::BeginDestroy()
{
	Disconnect();
	Super::BeginDestroy();
}
