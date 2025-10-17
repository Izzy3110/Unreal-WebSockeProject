// Copyright Athian Games. All Rights Reserved. 


#include "MySQLAsyncTasks.h"
#include "MySQLDBConnectionActor.h"
#include "MySQLBPLibrary.h"
#include "Async/Async.h"


OpenMySQLConnectionTask::OpenMySQLConnectionTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, int32 connectionID,
	TWeakObjectPtr<UMySQLDBConnector> dbConnector, FString server, FString dBName, FString userID, FString password, int32 port, TArray<FMySQLOptionPair> options)
{
	Server = server;
	DBName = dBName;
	UserID = userID;
	Password = password;
	Port = port;
	CurrentDBConnectionActor = dbConnectionActor;
	MySQLDBConnector = dbConnector;
	ConnectionID = connectionID;
	MySQLOptions = options;
}

OpenMySQLConnectionTask::~OpenMySQLConnectionTask()
{

}

void OpenMySQLConnectionTask::DoWork()
{
	if (MySQLDBConnector.IsValid())
	{
		MySQLDBConnector->CloseConnection(ConnectionID);

		FString ErrorMessage;
		bool ConnectionStatus = MySQLDBConnector->CreateNewConnection(ConnectionID, Server, DBName, UserID, Password, Port, MySQLOptions, ErrorMessage);
        
		AsyncTask(ENamedThreads::GameThread, [this, ConnectionStatus, ErrorMessage]()
		{
			if (CurrentDBConnectionActor.IsValid())
			{
				CurrentDBConnectionActor->bIsConnectionBusy = false;
				if(!ConnectionStatus)
				{
					CurrentDBConnectionActor->ResetLastConnection();
				}
                
				// Call the event for the server
				CurrentDBConnectionActor->OnConnectionStateChanged(ConnectionStatus, ConnectionID, ErrorMessage);
                
				// Check if this connection was requested by a client
				if (CurrentDBConnectionActor->ClientRequestMap.Contains(ConnectionID))
				{
					// Find the client actor
					FClientRequest ClientRequest = CurrentDBConnectionActor->ClientRequestMap[ConnectionID];
					APlayerController* Client = Cast<APlayerController>(ClientRequest.RequestingClient);
                    
					if (Client)
					{
						// Send the connection result back to the client
						CurrentDBConnectionActor->ClientReceiveConnectionStatus(Client, ConnectionStatus, ConnectionID, ErrorMessage);
					}
                    
					// Remove from the map
					CurrentDBConnectionActor->ClientRequestMap.Remove(ConnectionID);
				}
			}
		});
	}
}


UpdateMySQLQueryAsyncTask::UpdateMySQLQueryAsyncTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, TWeakObjectPtr<UMySQLDBConnector> dbConnector, int32 connectionID, int32 queryID, TArray<FString> queries)
{
	Queries = queries;
	CurrentDBConnectionActor = dbConnectionActor;
	MySQLDBConnector = dbConnector;
	ConnectionID = connectionID;
	QueryID = queryID;
}

UpdateMySQLQueryAsyncTask::~UpdateMySQLQueryAsyncTask()
{

}

void UpdateMySQLQueryAsyncTask::DoWork()
{
	FString ErrorMessage;
	bool currentUpdateQueryStatus = false;
    
	if (MySQLDBConnector.IsValid() && MySQLDBConnector->IsValidLowLevel())
	{
		for (int iIndex = 0; iIndex < Queries.Num(); iIndex++)
		{
			MySQLDBConnector->UpdateDataFromQuery(ConnectionID, QueryID, Queries[iIndex], currentUpdateQueryStatus, ErrorMessage);
		}
	}

	AsyncTask(ENamedThreads::GameThread, [this, currentUpdateQueryStatus, ErrorMessage]()
	{
		if (CurrentDBConnectionActor.IsValid() && CurrentDBConnectionActor->IsValidLowLevel())
		{
			CurrentDBConnectionActor->bIsConnectionBusy = false;
            
			// Forward results to server event handler
			CurrentDBConnectionActor->OnQueryUpdateStatusChanged(ConnectionID, QueryID, currentUpdateQueryStatus, ErrorMessage);
            
			// Check if this query was requested by a client
			if (CurrentDBConnectionActor->ClientRequestMap.Contains(QueryID))
			{
				// Send results back to the client
				APlayerController* Client = Cast<APlayerController>(CurrentDBConnectionActor->ClientRequestMap[QueryID].RequestingClient);
				if (Client)
				{
					CurrentDBConnectionActor->ClientReceiveUpdateStatus(ConnectionID, QueryID, currentUpdateQueryStatus, ErrorMessage);
				}
                
				// Clean up
				CurrentDBConnectionActor->ClientRequestMap.Remove(QueryID);
			}
            
			CurrentDBConnectionActor->ExecuteNextQueryTask();
		}
	});
}

SelectMySQLQueryAsyncTask::SelectMySQLQueryAsyncTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, TWeakObjectPtr<UMySQLDBConnector> dbConnector, int32 connectionID, int32 queryID, FString query)
{
	Query = query;
	CurrentDBConnectionActor = dbConnectionActor;
	MySQLDBConnector = dbConnector;
	ConnectionID = connectionID;
	QueryID = queryID;
}

SelectMySQLQueryAsyncTask::~SelectMySQLQueryAsyncTask()
{
	
}

void SelectMySQLQueryAsyncTask::DoWork()
{
	FString ErrorMessage;
	bool SelectQueryStatus;
	TArray<FMySQLDataTable> ResultByColumn;
	TArray<FMySQLDataRow> ResultByRow;

	if (MySQLDBConnector.IsValid())
	{
		MySQLDBConnector->SelectDataFromQuery(ConnectionID, Query, SelectQueryStatus, ErrorMessage, ResultByColumn, ResultByRow);
	}
	else
	{
		ErrorMessage = "Invalid Connection";
		SelectQueryStatus = false;
	}

	AsyncTask(ENamedThreads::GameThread, [this, SelectQueryStatus, ErrorMessage, ResultByColumn, ResultByRow]()
	{
		if (CurrentDBConnectionActor.IsValid() && CurrentDBConnectionActor->IsValidLowLevel())
		{
			CurrentDBConnectionActor->bIsConnectionBusy = false;
            
			// Forward results to server event handler
			CurrentDBConnectionActor->OnQuerySelectStatusChanged(ConnectionID, QueryID, SelectQueryStatus, ErrorMessage, ResultByColumn, ResultByRow);
            
			// Check if this query was requested by a client
			if (CurrentDBConnectionActor->ClientRequestMap.Contains(QueryID))
			{
				// Send results back to the client
				APlayerController* Client = Cast<APlayerController>(CurrentDBConnectionActor->ClientRequestMap[QueryID].RequestingClient);
				if (Client)
				{
					CurrentDBConnectionActor->ClientReceiveQueryResults(ConnectionID, QueryID, SelectQueryStatus, ErrorMessage, ResultByColumn, ResultByRow);
				}
                
				// Clean up
				CurrentDBConnectionActor->ClientRequestMap.Remove(QueryID);
			}
            
			CurrentDBConnectionActor->ExecuteNextQueryTask();
		}
	});
}


UpdateMySQLImageAsyncTask::UpdateMySQLImageAsyncTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, TWeakObjectPtr<UMySQLDBConnector> dbConnector, int32 connectionID, int32 queryID, FString query, FString updateParameter, int parameterID, FString imagePath)
{
	Query = query;
	UpdateParameter = updateParameter;
	ParameterID = parameterID;
	ImagePath = imagePath;
	CurrentDBConnectionActor = dbConnectionActor;
	MySQLDBConnector = dbConnector;
	ConnectionID = connectionID;
	QueryID = queryID;

}

UpdateMySQLImageAsyncTask::~UpdateMySQLImageAsyncTask()
{

}

void UpdateMySQLImageAsyncTask::DoWork()
{
	bool UpdateQueryStatus;
	FString ErrorMessage;

	if (MySQLDBConnector.IsValid())
	{
		 MySQLDBConnector->UpdateImageFromPath(ConnectionID, QueryID, Query, UpdateParameter, ParameterID, ImagePath, UpdateQueryStatus, ErrorMessage);
	}
	else
	{
		ErrorMessage = "InValid Connection";
		UpdateQueryStatus = false;
	}
	
	AsyncTask(ENamedThreads::GameThread, [this, UpdateQueryStatus, ErrorMessage]()
	{
		if (CurrentDBConnectionActor.IsValid() && CurrentDBConnectionActor->IsValidLowLevel())
		{
			CurrentDBConnectionActor->bIsConnectionBusy = false;
			CurrentDBConnectionActor->OnImageUpdateStatusChanged(ConnectionID, QueryID, UpdateQueryStatus, ErrorMessage);
        
			// Check if this was a client request
			if (CurrentDBConnectionActor->ClientRequestMap.Contains(QueryID))
			{
				APlayerController* Client = Cast<APlayerController>(CurrentDBConnectionActor->ClientRequestMap[QueryID].RequestingClient);
				if (Client)
				{
					CurrentDBConnectionActor->ClientReceiveImageUpdateStatus(ConnectionID, QueryID, UpdateQueryStatus, ErrorMessage);
				}
				CurrentDBConnectionActor->ClientRequestMap.Remove(QueryID);
			}
        
			CurrentDBConnectionActor->ExecuteNextQueryTask();
		}
	});
}


SelectMySQLImageAsyncTask::SelectMySQLImageAsyncTask(TWeakObjectPtr<AMySQLDBConnectionActor> dbConnectionActor, TWeakObjectPtr<UMySQLDBConnector> dbConnector,
	int32 connectionID, int32 queryID, FString query)
{
	Query = query;
	CurrentDBConnectionActor = dbConnectionActor;
	MySQLDBConnector = dbConnector;
	ConnectionID = connectionID;
	QueryID = queryID;

}

SelectMySQLImageAsyncTask::~SelectMySQLImageAsyncTask()
{

}

void SelectMySQLImageAsyncTask::DoWork()
{
	bool SelectQueryStatus;
	FString ErrorMessage;
	UTexture2D* SelectedTexture = nullptr;

	if (MySQLDBConnector.IsValid())
	{
		SelectedTexture = MySQLDBConnector->SelectImageFromQuery(ConnectionID, QueryID, Query, SelectQueryStatus, ErrorMessage);
	}
	else
	{
		ErrorMessage = "InValid Connection";
		SelectQueryStatus = false;
	}
	
	AsyncTask(ENamedThreads::GameThread, [this, SelectQueryStatus, ErrorMessage, SelectedTexture]()
{
	if (CurrentDBConnectionActor.IsValid() && CurrentDBConnectionActor->IsValidLowLevel())
	{
		CurrentDBConnectionActor->bIsConnectionBusy = false;
		CurrentDBConnectionActor->OnImageSelectStatusChanged(ConnectionID, QueryID, SelectQueryStatus, ErrorMessage, SelectedTexture);
        
		// Check if this was a client request
		if (CurrentDBConnectionActor->ClientRequestMap.Contains(QueryID))
		{
			APlayerController* Client = Cast<APlayerController>(CurrentDBConnectionActor->ClientRequestMap[QueryID].RequestingClient);
			if (Client)
			{
				CurrentDBConnectionActor->ClientReceiveImageSelectStatus(ConnectionID, QueryID, SelectQueryStatus, ErrorMessage, SelectedTexture);
			}
			CurrentDBConnectionActor->ClientRequestMap.Remove(QueryID);
		}
        
		CurrentDBConnectionActor->ExecuteNextQueryTask();
	}
});

}





