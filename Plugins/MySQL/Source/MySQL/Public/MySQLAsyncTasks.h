// Copyright Athian Games. All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Async/AsyncWork.h"
#include "Engine/DataTable.h"


#include "MySQLDBConnector.h"

/**
 * 
 */

class AMySQLDBConnectionActor;

class MYSQL_API OpenMySQLConnectionTask : public FNonAbandonableTask
{

	FString Server;
	FString DBName;
	FString UserID;
	FString Password;
	int32 Port;
	FString ExtraParam;
	TWeakObjectPtr<AMySQLDBConnectionActor> CurrentDBConnectionActor;
	TWeakObjectPtr<UMySQLDBConnector> MySQLDBConnector;
	TArray<FMySQLOptionPair> MySQLOptions;

public:

	int32 ConnectionID;

	OpenMySQLConnectionTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, int32 connectionID, TWeakObjectPtr<UMySQLDBConnector> dbConnector,
		FString server, FString dBName, FString userID, FString password, int32 Port, TArray<FMySQLOptionPair> options);

	virtual ~OpenMySQLConnectionTask();
	virtual void DoWork();
	
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(OpenConnectionTask, STATGROUP_ThreadPoolAsyncTasks);
	}

};


class MYSQL_API UpdateMySQLQueryAsyncTask : public FNonAbandonableTask
{

	TArray<FString> Queries;
	TWeakObjectPtr<AMySQLDBConnectionActor> CurrentDBConnectionActor;
	TWeakObjectPtr<UMySQLDBConnector> MySQLDBConnector;
	int32 ConnectionID;
	int32 QueryID;
	
public:


	UpdateMySQLQueryAsyncTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, TWeakObjectPtr<UMySQLDBConnector> dbConnector, int32 connectionID, int32 queryID,
		TArray<FString> queries);

	virtual ~UpdateMySQLQueryAsyncTask();
	virtual void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UpdateQueryAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

};


class MYSQL_API SelectMySQLQueryAsyncTask : public FNonAbandonableTask
{

	FString Query;
	TWeakObjectPtr<AMySQLDBConnectionActor> CurrentDBConnectionActor;
	TWeakObjectPtr<UMySQLDBConnector> MySQLDBConnector;
	int32 ConnectionID;
	int32 QueryID;
	
public:



	SelectMySQLQueryAsyncTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, TWeakObjectPtr<UMySQLDBConnector> dbConnector, int32 connectionID, int32 queryID, FString query);
	virtual ~SelectMySQLQueryAsyncTask();
	virtual void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(SelectQueryAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

};


class MYSQL_API UpdateMySQLImageAsyncTask : public FNonAbandonableTask
{

private:

	FString Query;
	TWeakObjectPtr<AMySQLDBConnectionActor> CurrentDBConnectionActor;
	FString UpdateParameter;
	int ParameterID;
	FString ImagePath;
	TWeakObjectPtr<UMySQLDBConnector> MySQLDBConnector;
	
	int32 ConnectionID;
	int32 QueryID;

public:


	UpdateMySQLImageAsyncTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, TWeakObjectPtr<UMySQLDBConnector> dbConnector, int32 connectionID, int32 queryID, FString query, FString updateParameter, int parameterID, FString imagePath);

	virtual ~UpdateMySQLImageAsyncTask();
	virtual void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UpdateImageAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

};


class MYSQL_API SelectMySQLImageAsyncTask : public FNonAbandonableTask
{

private:

	FString Query;
	TWeakObjectPtr<AMySQLDBConnectionActor> CurrentDBConnectionActor;
	TWeakObjectPtr<UMySQLDBConnector> MySQLDBConnector;

	int32 ConnectionID;
	int32 QueryID;

public:


	SelectMySQLImageAsyncTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, TWeakObjectPtr<UMySQLDBConnector> dbConnector, int32 connectionID,
		int32 queryID, FString query);

	virtual ~SelectMySQLImageAsyncTask();
	virtual void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(SelectImageAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

};




