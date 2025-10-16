// Must be first for explicit PCH
#include "Postgres.h"
#include "Misc/ScopeLock.h"
#include "Logging/LogMacros.h"
#include "PostgresClient.h"
#include "Async/Async.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Containers/StringConv.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPath.h"

THIRD_PARTY_INCLUDES_START
#include "libpq-fe.h"
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY_STATIC(LogPostgres, Log, All);

[[maybe_unused]]
static FString VecToPgArray(const FVector& V)
{
	// Compact formatting; Postgres will parse text arrays fine, we cast to float8[]
	return FString::Printf(TEXT("{%g,%g,%g}"), V.X, V.Y, V.Z);
}

// Unique helper name to avoid clashing with UE::MakeError
static FPostgresQueryResult MakePgError(const FString& Message)
{
	FPostgresQueryResult R;
	R.bSuccess = false;
	R.Error = Message;
	return R;
}

AActor* UPostgresClient::GetEntityActorFromDB(
    UObject* WorldContextObject,
    const FString& LevelName,
    ESpawnActorCollisionHandlingMethod CollisionHandlingOverride)
{
    FScopeLock Lock(&ConnMutex);

    if (!WorldContextObject)
    {
        UE_LOG(LogPostgres, Error, TEXT("GetEntityActorFromDB: WorldContextObject is null"));
        return nullptr;
    }
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObjectChecked(WorldContextObject) : nullptr;
    if (!World)
    {
        UE_LOG(LogPostgres, Error, TEXT("GetEntityActorFromDB: World is null"));
        return nullptr;
    }

    if (!Conn || PQstatus(Conn) != CONNECTION_OK)
    {
        UE_LOG(LogPostgres, Error, TEXT("GetEntityActorFromDB: not connected."));
        return nullptr;
    }

    const FString Sql =
        TEXT("SELECT ")
        TEXT("  class_name, ")
        TEXT("  world_location[1], world_location[2], world_location[3], ")
        TEXT("  world_rotation[1], world_rotation[2], world_rotation[3], ")
        TEXT("  world_scale[1],    world_scale[2],    world_scale[3] ")
        TEXT("FROM entities ")
        TEXT("WHERE level_name = $1 ")
        TEXT("ORDER BY created_at DESC ")
        TEXT("LIMIT 1;");

    const FTCHARToUTF8 SqlUtf8(*Sql);
    const FTCHARToUTF8 P0(*LevelName);
	
    const char* Params[1] = { P0.Get() };

    PGresult* Res = PQexecParams(
        Conn, SqlUtf8.Get(),
        1, nullptr, Params, nullptr, nullptr, 0);

    if (!Res)
    {
        UE_LOG(LogPostgres, Error, TEXT("GetEntityActorFromDB: PQExecParams null: %hs"),
               PQerrorMessage(Conn));
        return nullptr;
    }

    const ExecStatusType Status = PQresultStatus(Res);
    if (Status != PGRES_TUPLES_OK || PQntuples(Res) < 1)
    {
        UE_LOG(LogPostgres, Warning, TEXT("GetEntityActorFromDB: no row for '%s' (%hs)"),
               *LevelName, PQresultErrorMessage(Res));
        PQclear(Res);
        return nullptr;
    }

	auto F = [&](int32 C) -> float
	{
		float V = 0.f;
		LexFromString(V, UTF8_TO_TCHAR(PQgetvalue(Res, 0, C)));
		return V;
	};

    const FString ClassPath = UTF8_TO_TCHAR(PQgetvalue(Res,0,0)); // expects "/Game/.../BP_X.BP_X_C"

    const FVector Location(F(1), F(2), F(3));
    const FRotator Rotator (F(4), F(5), F(6)); // Pitch,Yaw,Roll (degrees)
    const FVector Scale   (F(7), F(8), F(9));

    PQclear(Res);

    // Load class and spawn
    FSoftClassPath Scp(ClassPath);
    UClass* Cls = Scp.TryLoadClass<AActor>();
    if (!Cls)
    {
        UE_LOG(LogPostgres, Error, TEXT("GetEntityActorFromDB: failed to load class '%s'"), *ClassPath);
        return nullptr;
    }

    FActorSpawnParameters ParamsSpawn;
    ParamsSpawn.SpawnCollisionHandlingOverride = CollisionHandlingOverride;

    const FTransform XTransform(Rotator, Location, Scale);
    return World->SpawnActor<AActor>(Cls, XTransform, ParamsSpawn);
}

bool UPostgresClient::AddEntity(
    const FString& LevelName,
    const FString& ClassName,
    FVector LocalRotation,
    FVector LocalLocation,
    FVector WorldRotation,
    FVector WorldLocation,
    FVector WorldScale)
{
    FScopeLock Lock(&ConnMutex);

    if (!Conn || PQstatus(Conn) != CONNECTION_OK)
    {
        UE_LOG(LogPostgres, Error, TEXT("AddEntity: not connected."));
        return false;
    }

    // Ensure column exists once:
    // ALTER TABLE entities ADD COLUMN IF NOT EXISTS world_scale DOUBLE PRECISION[3] NOT NULL DEFAULT ARRAY[1,1,1]::float8[];

    const FString Sql =
        TEXT("INSERT INTO entities ")
        TEXT("(level_name, class_name, ")
        TEXT(" local_rotation, local_location, ")
        TEXT(" world_rotation, world_location, world_scale, ")
        TEXT(" created_at, moved_at) ")
        TEXT("VALUES (")
        TEXT("  $1, $2, ")
        TEXT("  ARRAY[$3,$4,$5]::float8[], ")
        TEXT("  ARRAY[$6,$7,$8]::float8[], ")
        TEXT("  ARRAY[$9,$10,$11]::float8[], ")
        TEXT("  ARRAY[$12,$13,$14]::float8[], ")
        TEXT("  ARRAY[$15,$16,$17]::float8[], ")
        TEXT("  now(), NULL);");

    auto AddVec = [](TArray<FString>& Out, const FVector& V)
    {
        // LexToString uses '.' and is locale-independent
        Out.Add(LexToString(V.X));
        Out.Add(LexToString(V.Y));
        Out.Add(LexToString(V.Z));
    };

    // 2 strings + 15 float components
    TArray<FString> Params;
    Params.Reserve(17);
    Params.Add(LevelName);
    Params.Add(ClassName);
    AddVec(Params, LocalRotation);
    AddVec(Params, LocalLocation);
    AddVec(Params, WorldRotation);
    AddVec(Params, WorldLocation);
    AddVec(Params, WorldScale);

    // Convert to UTF-8 for lib
    TArray<FTCHARToUTF8> Converters;
    Converters.Reserve(Params.Num());
    TArray<const char*> ParamPointers;
    ParamPointers.SetNum(Params.Num());
    for (int32 i = 0; i < Params.Num(); ++i)
    {
        Converters.Emplace(*Params[i]);
        ParamPointers[i] = Converters.Last().Get();
    }

    const FTCHARToUTF8 SqlUtf8(*Sql);
    PGresult* Res = PQexecParams(
        Conn,
        SqlUtf8.Get(),
        Params.Num(),          // 17
        nullptr,               // infer types (we cast arrays in SQL)
        ParamPointers.GetData(),
        nullptr,
        nullptr,
        0
    );

    if (!Res)
    {
        UE_LOG(LogPostgres, Error, TEXT("AddEntity: PQExecParams returned null: %hs"), PQerrorMessage(Conn));
        return false;
    }

    const ExecStatusType Status = PQresultStatus(Res);
    if (Status != PGRES_COMMAND_OK)
    {
    	UE_LOG(LogPostgres, Error, TEXT("PQ error: %hs"), PQresultErrorMessage(Res));
        PQclear(Res);
        return false;
    }

    PQclear(Res);
    return true;
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
		UE_LOG(LogPostgres, Error, TEXT("lib preload failed. See earlier [Postgres] log for missing DLL(s)."));
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
		FString Err = Conn ? UTF8_TO_TCHAR(PQerrorMessage(Conn)) : TEXT("connect returned null.");
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

/*
FPostgresQueryResult UPostgresClient::ExecParams(const FString& Sql, const TArray<FString>& Params)
{
	return ExecInternal(Sql, &Params);
}
*/

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
		return ::MakePgError(TEXT("Not connected to PostgresQL."));
	}

	FScopeLock Lock(&ConnMutex);

	FTCHARToUTF8 SqlUtf8(*Sql);
	PGresult* PgRes; // note: no initializer

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
		return ::MakePgError(TEXT("exec returned null."));
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
