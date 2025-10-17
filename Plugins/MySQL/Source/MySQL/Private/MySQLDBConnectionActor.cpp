// Copyright, Athian Games. All Rights Reserved. 

#include "MySQLDBConnectionActor.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMySQLDBConnectionActor::AMySQLDBConnectionActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bIsConnectionBusy = false;
    
    // Set up replication
    bReplicates = true;
    bAlwaysRelevant = true;
    
    // Default to server-only mode for safety
    ReplicationMode = EMySQLReplicationMode::ServerOnly;

    CopyDLL(TEXT("mysqlcppconn-9-vs14.dll"));
    CopyDLL(TEXT("libcrypto-1_1-x64.dll"));
    CopyDLL(TEXT("libssl-1_1-x64.dll"));
}

// Called when the game starts or when spawned
void AMySQLDBConnectionActor::BeginPlay()
{
    CloseAllConnections();
    Super::BeginPlay();
}

FCriticalSection Mutex;

// Called every frame
void AMySQLDBConnectionActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Mutex.Lock(); // Lock access to shared resources
    // Clean up finished tasks
    CleanUpFinishedTasks<OpenMySQLConnectionTask>(OpenConnectionTasks);
    CleanUpFinishedTasks<UpdateMySQLQueryAsyncTask>(UpdateQueryTasks);
    CleanUpFinishedTasks<SelectMySQLQueryAsyncTask>(SelectQueryTasks);
    CleanUpFinishedTasks<UpdateMySQLImageAsyncTask>(UpdateImageQueryTasks);
    CleanUpFinishedTasks<SelectMySQLImageAsyncTask>(SelectImageQueryTasks);
    Mutex.Unlock(); // Unlock access to shared resources
    
    // Update the busy state
    bIsConnectionBusy = OpenConnectionTasks.Num() > 0
        || UpdateQueryTasks.Num() > 0
        || SelectQueryTasks.Num() > 0
        || UpdateImageQueryTasks.Num() > 0
        || SelectImageQueryTasks.Num() > 0;

    if (!bIsConnectionBusy)
    {
        for (auto& entry : ConnectionToNextQueryIDMap)
        {
            entry.Value = 0;
        }
    }
}

void AMySQLDBConnectionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Ensure all UpdateQueryTasks have completed
    for(const auto& Task : UpdateQueryTasks)
    {
        if(Task && !Task->IsDone())
        {
            Task->EnsureCompletion();
        }
    }
    UpdateQueryTasks.Empty();

    // Ensure all SelectQueryTasks have completed
    for(const auto& Task : SelectQueryTasks)
    {
        if(Task && !Task->IsDone())
        {
            Task->EnsureCompletion();
        }
    }
    SelectQueryTasks.Empty();

    // Now you can safely close all connections
    CloseAllConnections();

    Super::EndPlay(EndPlayReason);
}

// Replication setup
void AMySQLDBConnectionActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AMySQLDBConnectionActor, ReplicationMode);
}

void AMySQLDBConnectionActor::OnRep_ReplicationModeChanged()
{
    // Log or handle replication mode changes if needed
    UE_LOG(LogTemp, Display, TEXT("MySQL Replication Mode changed to: %d"), static_cast<int32>(ReplicationMode));
}

bool AMySQLDBConnectionActor::CanExecuteQueryInCurrentContext() const
{
    switch (ReplicationMode)
    {
    case EMySQLReplicationMode::ServerOnly:
        return HasAuthority();
           
    case EMySQLReplicationMode::ClientToServer:
        return HasAuthority();
           
    case EMySQLReplicationMode::Multicast:
        return true;
           
    default:
        return false;
    }
}

bool AMySQLDBConnectionActor::ServerExecuteQuery_Validate(const FQueryRequest& QueryRequest)
{
    // Basic validation
    return true;
}

void AMySQLDBConnectionActor::ClientReceiveConnectionStatus_Implementation(APlayerController* Client, bool ConnectionStatus, int32 ConnectionID, const FString& ErrorMessage)
{
    // Only process if we're the target client
    if (Client == GetWorld()->GetFirstPlayerController())
    {
        UE_LOG(LogTemp, Log, TEXT("CLIENT: Received connection status: %s, ID: %d"), 
               ConnectionStatus ? TEXT("Connected") : TEXT("Failed"), ConnectionID);
        
        // Update the client's connection map
        if (ConnectionStatus)
        {
            // Create a dummy connector to keep track of the connection
            UMySQLDBConnector* DummyConnector = NewObject<UMySQLDBConnector>(this);
            SQLConnectors.Add(ConnectionID, DummyConnector);
            
            // Make sure we have an entry in the query ID map
            if (!ConnectionToNextQueryIDMap.Contains(ConnectionID))
            {
                ConnectionToNextQueryIDMap.Add(ConnectionID, 0);
            }
        }
        
        // Forward to the event
        OnConnectionStateChanged(ConnectionStatus, ConnectionID, ErrorMessage);
    }
}

void AMySQLDBConnectionActor::ServerExecuteQuery_Implementation(const FQueryRequest& QueryRequest)
{
    // Server executes the query on behalf of the client
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("ServerExecuteQuery called on client - this should never happen"));
        return; // Safety check
    }
    
    UE_LOG(LogTemp, Log, TEXT("Server received query request from client"));
    
    // Handle special connection request
    if (QueryRequest.ConnectionID == -1 && QueryRequest.QueryString.StartsWith(TEXT("CREATE_CONNECTION|")))
    {
        TArray<FString> Params;
        QueryRequest.QueryString.ParseIntoArray(Params, TEXT("|"), true);
        
        if (Params.Num() >= 6)
        {
            FString Server = Params[1];
            FString DBName = Params[2];
            FString UserID = Params[3];
            FString Password = Params[4];
            int32 Port = FCString::Atoi(*Params[5]);
            
            UE_LOG(LogTemp, Log, TEXT("Server creating connection: %s, %s, %s"), *Server, *DBName, *UserID);
            
            // Get the client that sent the request
            APlayerController* Client = Cast<APlayerController>(GetOwner());
            if (!Client && GetOwner())
            {
                // Try to find the player controller if owner isn't directly a controller
                Client = GetOwner()->GetWorld()->GetFirstPlayerController();
            }
            
            // Create the actual connection
            int32 ConnectionID;
            UMySQLDBConnector* NewConnector = CreateDBConnector(ConnectionID);

            TArray<FMySQLOptionPair> MySQLOptions;
            if(MySQLOptionsAsset)
            {
                MySQLOptions = MySQLOptionsAsset->ConnectionOptions;
            }
            
            // Create the connection and store the requesting client info
            FAsyncTask<OpenMySQLConnectionTask>* OpenConnectionTask = StartAsyncTask<OpenMySQLConnectionTask>(
                this, ConnectionID, NewConnector, Server, DBName, UserID, Password, Port, MySQLOptions);
            
            OpenConnectionTasks.Add(OpenConnectionTask);
            
            // Store client info to send result back when connection completes
            if (Client)
            {
                FClientRequest ClientRequest;
                ClientRequest.QueryID = -1; // Special value for connection
                ClientRequest.RequestingClient = Cast<AActor>(Client);
                ClientRequestMap.Add(ConnectionID, ClientRequest);
            }
            
            return;
        }
    }
    
    // Regular query handling
    int32 QueryID = GenerateQueryID(QueryRequest.ConnectionID);
    
    // Store the client request for sending back results
    APlayerController* Client = Cast<APlayerController>(GetOwner());
    if (Client)
    {
        FClientRequest ClientRequest;
        ClientRequest.QueryID = QueryID;
        ClientRequest.RequestingClient = Client;
        ClientRequestMap.Add(QueryID, ClientRequest);
    }
    
    if (QueryRequest.bIsSelectQuery)
    {
        // Execute a select query
        SelectDataFromQuery(QueryRequest.ConnectionID, QueryRequest.QueryString);
    }
    else
    {
        // Execute an update query
        UpdateDataFromQuery(QueryRequest.ConnectionID, QueryRequest.QueryString);
    }
}

bool AMySQLDBConnectionActor::ServerExecuteImageQuery_Validate(const FQueryRequest& QueryRequest)
{
    // Basic validation
    return true;
}

void AMySQLDBConnectionActor::ServerExecuteImageQuery_Implementation(const FQueryRequest& QueryRequest)
{
    // Server executes the image query on behalf of the client
    if (!HasAuthority())
    {
        return; // Safety check
    }
    
    int32 QueryID = GenerateQueryID(QueryRequest.ConnectionID);
    
    // Store the client request for sending back results
    FClientRequest ClientRequest;
    ClientRequest.QueryID = QueryID;
    ClientRequest.RequestingClient = GetOwner();
    ClientRequestMap.Add(QueryID, ClientRequest);
    
    if (QueryRequest.bIsSelectQuery)
    {
        // Execute a select image query
        SelectImageFromQuery(QueryRequest.ConnectionID, QueryRequest.QueryString);
    }
    else
    {
        // Execute an update image query
        UpdateImageFromPath(QueryRequest.ConnectionID, QueryRequest.QueryString, 
                           QueryRequest.UpdateParameter, QueryRequest.ParameterID, 
                           QueryRequest.ImagePath);
    }
}

// Client-side callback implementations
void AMySQLDBConnectionActor::ClientReceiveQueryResults_Implementation(int32 ConnectionID, int32 QueryID, bool IsSuccessful, 
                            const FString& ErrorMessage, const TArray<FMySQLDataTable>& ResultByColumn, 
                            const TArray<FMySQLDataRow>& ResultByRow)
{
    // Forward the results to the blueprint event
    OnQuerySelectStatusChanged(ConnectionID, QueryID, IsSuccessful, ErrorMessage, ResultByColumn, ResultByRow);
}

void AMySQLDBConnectionActor::ClientReceiveUpdateStatus_Implementation(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage)
{
    // Forward the status to the blueprint event
    OnQueryUpdateStatusChanged(ConnectionID, QueryID, IsSuccessful, ErrorMessage);
}

void AMySQLDBConnectionActor::ClientReceiveImageSelectStatus_Implementation(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage, UTexture2D* SelectedTexture)
{
    // Forward the status to the blueprint event
    OnImageSelectStatusChanged(ConnectionID, QueryID, IsSuccessful, ErrorMessage, SelectedTexture);
}

void AMySQLDBConnectionActor::ClientReceiveImageUpdateStatus_Implementation(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage)
{
    // Forward the status to the blueprint event
    OnImageUpdateStatusChanged(ConnectionID, QueryID, IsSuccessful, ErrorMessage);
}



int32 AMySQLDBConnectionActor::GenerateQueryID(int32 ConnectionID)
{
    if (!ConnectionToNextQueryIDMap.Contains(ConnectionID))
    {
        ConnectionToNextQueryIDMap.Add(ConnectionID, 0);
    }

    const int32 QueryID = ConnectionToNextQueryIDMap[ConnectionID];
    ConnectionToNextQueryIDMap[ConnectionID]++;
    return QueryID;
}

int32 AMySQLDBConnectionActor::GetLastQueryID(int32 ConnectionID)
{
    int32 QueryID = -1;
    if (ConnectionToNextQueryIDMap.Contains(ConnectionID))
    {
        QueryID = ConnectionToNextQueryIDMap[ConnectionID];
    }

    return QueryID;
}

UMySQLDBConnector* AMySQLDBConnectionActor::GetConnector(int32 ConnectionID)
{
    if (UMySQLDBConnector** ConnectorPtr = SQLConnectors.Find(ConnectionID))
    {
        if (UMySQLDBConnector* CurrentConnector = *ConnectorPtr)
        {
            return CurrentConnector;
        }
    }

    return nullptr;
}

void AMySQLDBConnectionActor::CloseConnection(int32 ConnectionID)
{
    if (!CanExecuteQueryInCurrentContext())
    {
        if (!HasAuthority() && ReplicationMode == EMySQLReplicationMode::ClientToServer)
        {
            // Create a close connection request
            FQueryRequest Request;
            Request.ConnectionID = ConnectionID;
            Request.bIsSelectQuery = false;
            Request.QueryString = TEXT("CLOSE_CONNECTION");
            
            ServerExecuteQuery(Request);
            return;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot close connection in current network context"));
            return;
        }
    }

    FQueryTaskData CloseConnectionTask;
    CloseConnectionTask.ConnectionID = ConnectionID;
    CloseConnectionTask.QueryType = EQueryType::Close;
    QueryTaskQueue.Add(CloseConnectionTask);

    if (!bIsQueryTaskRunning)
    {
        ExecuteNextQueryTask();
    }
}

void AMySQLDBConnectionActor::CloseAllConnections()
{
    if (!CanExecuteQueryInCurrentContext())
    {
        if (!HasAuthority() && ReplicationMode == EMySQLReplicationMode::ClientToServer)
        {
            // Create a close all connections request
            FQueryRequest Request;
            Request.ConnectionID = -1; // Special value for close all
            Request.bIsSelectQuery = false;
            Request.QueryString = TEXT("CLOSE_ALL_CONNECTIONS");
            
            ServerExecuteQuery(Request);
            return;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot close all connections in current network context"));
            return;
        }
    }

    TArray<int32> ConnectionKeys;
    SQLConnectors.GetKeys(ConnectionKeys);

    for (int iIndex = 0; iIndex < ConnectionKeys.Num(); iIndex++)
    {
        CloseConnection(ConnectionKeys[iIndex]);
    }

    SQLConnectors.Empty();
}

UMySQLDBConnector* AMySQLDBConnectionActor::CreateDBConnector(int32& ConnectionID)
{
    ConnectionID = SQLConnectors.Num();
    if (SQLConnectors.Num() > ConnectionID)
    {
        if (UMySQLDBConnector* NewConnector = SQLConnectors[ConnectionID])
        {
            NewConnector->CloseConnection(ConnectionID);
            NewConnector = nullptr;
            SQLConnectors.Remove(ConnectionID);
        }
    }

    UMySQLDBConnector* NewConnector = NewObject<UMySQLDBConnector>();
    SQLConnectors.Add(ConnectionID, NewConnector);
    return NewConnector;
}

void AMySQLDBConnectionActor::CopyDLL(FString DLLName)
{
    const FString Pluginpath = IPluginManager::Get().FindPlugin(TEXT("MySQL"))->GetBaseDir();
    const FString PluginDLLPath = FPaths::Combine(*Pluginpath, TEXT("Binaries"), TEXT("Win64"), DLLName);

    const FString ProjectDirectory = FPaths::ProjectDir();
    const FString ProjectDLLDirectory = FPaths::Combine(*ProjectDirectory, TEXT("Binaries"), TEXT("Win64"));

    const FString ProjectDLLPath = FPaths::Combine(*ProjectDLLDirectory, DLLName);

    if (!FPaths::DirectoryExists(*ProjectDLLDirectory))
    {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*ProjectDLLDirectory);
    }

    if (FPaths::FileExists(ProjectDLLPath))
    {
        FPlatformFileManager::Get().GetPlatformFile().CopyFile(*PluginDLLPath, *ProjectDLLPath);
    }
    else if (FPaths::FileExists(PluginDLLPath))
    {
        FPlatformFileManager::Get().GetPlatformFile().CopyFile(*ProjectDLLPath, *PluginDLLPath);
    }
}

void AMySQLDBConnectionActor::ResetLastConnection()
{
    const int32 ConnectionID = SQLConnectors.Num() - 1;
    if(ConnectionID >= 0)
    {
        if(UMySQLDBConnector* CurrentConnector = SQLConnectors[ConnectionID])
        {
            CurrentConnector->ConditionalBeginDestroy();
            CurrentConnector = nullptr;
        }
        SQLConnectors.Remove(ConnectionID);
    }
}

void AMySQLDBConnectionActor::CreateNewConnection(FString Server, FString DBName, FString UserID, FString Password, int32 Port)
{
    // Log the attempt for debugging
    UE_LOG(LogTemp, Warning, TEXT("Creating connection from %s: Server=%s, DB=%s, User=%s"), 
           HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *Server, *DBName, *UserID);

    if (!HasAuthority() && ReplicationMode == EMySQLReplicationMode::ClientToServer)
    {
        // More verbose client-side logging
        UE_LOG(LogTemp, Warning, TEXT("CLIENT: Forwarding connection request to server"));
        
        // Create a special request and send to server
        FQueryRequest Request;
        Request.ConnectionID = -1; // Special value for new connection
        Request.bIsSelectQuery = false;
        Request.QueryString = FString::Printf(TEXT("CREATE_CONNECTION|%s|%s|%s|%s|%d"), 
                                               *Server, *DBName, *UserID, *Password, Port);
        
        ServerExecuteQuery(Request);
        
        // IMPORTANT: Simulate a local connection ID for the client to use
        int32 SimulatedConnectionID = SQLConnectors.Num();
        ConnectionToNextQueryIDMap.Add(SimulatedConnectionID, 0);
        
        // Notify the client that a connection attempt is in progress
        OnConnectionStateChanged(true, SimulatedConnectionID, TEXT("Connection request sent to server"));
        return;
    }
    
    if (!HasAuthority() && ReplicationMode == EMySQLReplicationMode::ServerOnly)
    {
        // Notify the client that they can't create connections in this mode
        OnConnectionStateChanged(false, -1, TEXT("Cannot create connection in ServerOnly mode from client"));
        return;
    }
    
    // Original implementation for server or multicast mode
    int32 ConnectionID;
    UMySQLDBConnector* NewConnector = CreateDBConnector(ConnectionID);

    TArray<FMySQLOptionPair> MySQLOptions;
    if(MySQLOptionsAsset)
    {
        MySQLOptions = MySQLOptionsAsset->ConnectionOptions;
    }
    
    FAsyncTask<OpenMySQLConnectionTask>* OpenConnectionTask = StartAsyncTask<OpenMySQLConnectionTask>(
        this, ConnectionID, NewConnector, Server, DBName, UserID, Password, Port, MySQLOptions);
    
    OpenConnectionTasks.Add(OpenConnectionTask);
}

void AMySQLDBConnectionActor::ExecuteNextQueryTask()
{
    if(IsValidLowLevel())
    {
        if(QueryTaskQueue.Num() > 0)
        {
            FQueryTaskData& TaskData = QueryTaskQueue.Last();
            UMySQLDBConnector* CurrentConnector = GetConnector(TaskData.ConnectionID);
            if(CurrentConnector == nullptr)
            {
                UE_LOG(LogTemp, Warning, TEXT("CurrentConnector is null"));
                return;
            }

            bIsQueryTaskRunning = true;
            switch (TaskData.QueryType)
            {
            case EQueryType::Update:
                {
                    FAsyncTask<UpdateMySQLQueryAsyncTask>* UpdateQueryTask = StartAsyncTask<UpdateMySQLQueryAsyncTask>(this, CurrentConnector, TaskData.ConnectionID, TaskData.QueryID, TaskData.Queries);
                    if(UpdateQueryTask == nullptr)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("UpdateQueryTask is null"));
                        return;
                    }
                    UpdateQueryTasks.Add(UpdateQueryTask);
                }
                break;
            case EQueryType::Select:
                {
                    FAsyncTask<SelectMySQLQueryAsyncTask>* SelectQueryTask = StartAsyncTask<SelectMySQLQueryAsyncTask>(this, CurrentConnector, TaskData.ConnectionID, TaskData.QueryID, TaskData.Queries[0]);
                    if(SelectQueryTask == nullptr)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("SelectQueryTask is null"));
                        return;
                    }
                    SelectQueryTasks.Add(SelectQueryTask);
                }
                break;
            case EQueryType::Close:
                {
                    CurrentConnector->CloseConnection(TaskData.ConnectionID);
                    SQLConnectors.Remove(TaskData.ConnectionID);
                    ConnectionToNextQueryIDMap.Remove(TaskData.ConnectionID);
                    bIsQueryTaskRunning = false;
                }
                break;
            case EQueryType::Endplay:
                {
                    QueryTaskQueue.Empty();
                    CloseAllConnections();
                    Super::EndPlay(EEndPlayReason::Type::Quit);
                }
                break;
            default:
                break;
            }
            QueryTaskQueue.RemoveAt(QueryTaskQueue.Num() - 1);
        }
        else
        {
            bIsQueryTaskRunning = false;
        }
    }
}

void AMySQLDBConnectionActor::CreateTaskData(int32 ConnectionID, TArray<FString> Queries, EQueryType QueryType)
{
    // Create a struct with the query data and add it to the queue
    FQueryTaskData TaskData;
    TaskData.ConnectionID = ConnectionID;
    TaskData.QueryID = GenerateQueryID(ConnectionID);
    TaskData.Queries = Queries;
    TaskData.QueryType = QueryType;
    QueryTaskQueue.Add(TaskData);

    // If no task is currently running, execute the next task
    if (!bIsQueryTaskRunning)
    {
        ExecuteNextQueryTask();
    }
}

bool AMySQLDBConnectionActor::HandleQueryExecutionContext(int32 ConnectionID, EQueryExecutionContext ExecutionContext, 
                                                         bool bIsSelectQuery, const FString& Query, 
                                                         FString& OutErrorMessage)
{
    // Determine if context allows local execution
    bool bCanExecuteLocally = false;
    
    switch (ExecutionContext)
    {
        case EQueryExecutionContext::Default:
            bCanExecuteLocally = CanExecuteQueryInCurrentContext();
            if (!bCanExecuteLocally) 
            {
                OutErrorMessage = FString::Printf(TEXT("Cannot execute query in %s with ReplicationMode=%s"), 
                    HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
                    ReplicationMode == EMySQLReplicationMode::ServerOnly ? TEXT("ServerOnly") : 
                    ReplicationMode == EMySQLReplicationMode::ClientToServer ? TEXT("ClientToServer") : TEXT("Multicast"));
            }
            break;
            
        case EQueryExecutionContext::ForceServer:
            // Immediately fail if client with no client-to-server option
            if (!HasAuthority() && ReplicationMode != EMySQLReplicationMode::ClientToServer)
            {
                OutErrorMessage = TEXT("ForceServer queries can only run on server or via ClientToServer mode");
                return false; // Return false to indicate immediate failure
            }
            bCanExecuteLocally = HasAuthority();
            if (!bCanExecuteLocally) OutErrorMessage = TEXT("ForceServer execution context requires SERVER");
            break;
            
        case EQueryExecutionContext::ForceClient:
            // Immediately fail if server without multicast
            if (HasAuthority() && ReplicationMode != EMySQLReplicationMode::Multicast)
            {
                OutErrorMessage = TEXT("ForceClient queries cannot run on server except in Multicast mode");
                return false; // Return false to indicate immediate failure
            }
            bCanExecuteLocally = !HasAuthority() || ReplicationMode == EMySQLReplicationMode::Multicast;
            if (!bCanExecuteLocally) OutErrorMessage = TEXT("ForceClient execution context requires CLIENT or Multicast mode");
            break;
    }
    
    // If we can't execute locally, see if we can route to server
    if (!bCanExecuteLocally)
    {
        // Check if we can route to server
        if (!HasAuthority() && (ReplicationMode == EMySQLReplicationMode::ClientToServer || 
                               ExecutionContext == EQueryExecutionContext::ForceServer))
        {
            // Create request
            FQueryRequest Request;
            Request.ConnectionID = ConnectionID;
            Request.QueryString = Query;
            Request.bIsSelectQuery = bIsSelectQuery;
            
            // Send to server
            ServerExecuteQuery(Request);
            return false; // Signal that query was routed
        }
        
        // If we get here, we can't execute locally and can't route
        return false;
    }
    
    // If we get here, we can execute locally
    return true;
}

void AMySQLDBConnectionActor::UpdateDataFromQuery(int32 ConnectionID, FString Query, EQueryExecutionContext ExecutionContext)
{
    FString ErrorMessage;
    if (!HandleQueryExecutionContext(ConnectionID, ExecutionContext, false, Query, ErrorMessage))
    {
        // If false but no error, it was routed to server
        if (!ErrorMessage.IsEmpty())
        {
            OnQueryUpdateStatusChanged(ConnectionID, -1, false, ErrorMessage);
        }
        return;
    }
    
    // Execute locally
    TArray<FString> Queries;
    Queries.Add(Query);
    CreateTaskData(ConnectionID, Queries, EQueryType::Update);
}

void AMySQLDBConnectionActor::UpdateDataFromMultipleQueries(int32 ConnectionID, TArray<FString> Queries, EQueryExecutionContext ExecutionContext)
{
    FString ErrorMessage;
    bool bHandled = false;
    
    // For multiple queries, check each one individually
    if (!HasAuthority() && (ReplicationMode == EMySQLReplicationMode::ClientToServer || 
                           ExecutionContext == EQueryExecutionContext::ForceServer))
    {
        // Route each query to server
        for (const FString& Query : Queries)
        {
            FQueryRequest Request;
            Request.ConnectionID = ConnectionID;
            Request.QueryString = Query;
            Request.bIsSelectQuery = false;
            ServerExecuteQuery(Request);
        }
        bHandled = true;
    }
    else if (!CanExecuteQueryInCurrentContext())
    {
        ErrorMessage = FString::Printf(TEXT("Cannot execute multiple queries in current context (ReplicationMode=%s)"), 
            ReplicationMode == EMySQLReplicationMode::ServerOnly ? TEXT("ServerOnly") : 
            ReplicationMode == EMySQLReplicationMode::ClientToServer ? TEXT("ClientToServer") : TEXT("Multicast"));
    }
    
    if (bHandled)
    {
        return;
    }
    else if (!ErrorMessage.IsEmpty())
    {
        OnQueryUpdateStatusChanged(ConnectionID, -1, false, ErrorMessage);
        return;
    }
    
    // Execute locally
    CreateTaskData(ConnectionID, Queries, EQueryType::Update);
}


void AMySQLDBConnectionActor::SelectDataFromQuery(int32 ConnectionID, FString Query, EQueryExecutionContext ExecutionContext)
{
    FString ErrorMessage;
    if (!HandleQueryExecutionContext(ConnectionID, ExecutionContext, true, Query, ErrorMessage))
    {
        // If false but no error, it was routed to server
        if (!ErrorMessage.IsEmpty())
        {
            OnQuerySelectStatusChanged(ConnectionID, -1, false, ErrorMessage, 
                TArray<FMySQLDataTable>(), TArray<FMySQLDataRow>());
        }
        return;
    }
    
    // Execute locally
    TArray<FString> Queries;
    Queries.Add(Query);
    CreateTaskData(ConnectionID, Queries, EQueryType::Select);
}


void AMySQLDBConnectionActor::SelectImageFromQuery(int32 ConnectionID, FString Query, EQueryExecutionContext ExecutionContext)
{
    FString ErrorMessage;
    if (!HandleQueryExecutionContext(ConnectionID, ExecutionContext, true, Query, ErrorMessage))
    {
        // If false but no error, it means it was routed to server
        if (!ErrorMessage.IsEmpty())
        {
            OnImageSelectStatusChanged(ConnectionID, -1, false, ErrorMessage, nullptr);
        }
        return;
    }
    
    // Execute locally
    if (UMySQLDBConnector* CurrentConnector = GetConnector(ConnectionID))
    {
        int32 QueryID = GenerateQueryID(ConnectionID);
        FAsyncTask<SelectMySQLImageAsyncTask>* SelectImageQueryTask = StartAsyncTask<SelectMySQLImageAsyncTask>(
            this, CurrentConnector, ConnectionID, QueryID, Query);
        SelectImageQueryTasks.Add(SelectImageQueryTask);
    }
}

void AMySQLDBConnectionActor::UpdateImageFromPath(int32 ConnectionID, FString Query, FString UpdateParameter, 
                                                 int ParameterID, FString ImagePath, EQueryExecutionContext ExecutionContext)
{
    FString ErrorMessage;
    if (!HandleQueryExecutionContext(ConnectionID, ExecutionContext, false, Query, ErrorMessage))
    {
        // Special case for image queries - need to pass additional parameters
        if (ErrorMessage.IsEmpty() && !HasAuthority())
        {
            // Create specialized image request
            FQueryRequest Request;
            Request.ConnectionID = ConnectionID;
            Request.QueryString = Query;
            Request.UpdateParameter = UpdateParameter;
            Request.ParameterID = ParameterID;
            Request.ImagePath = ImagePath;
            Request.bIsSelectQuery = false;
            Request.bIsImageQuery = true;
            
            ServerExecuteImageQuery(Request);
        }
        else if (!ErrorMessage.IsEmpty())
        {
            OnImageUpdateStatusChanged(ConnectionID, -1, false, ErrorMessage);
        }
        return;
    }
    
    // Execute locally
    if (UMySQLDBConnector* CurrentConnector = GetConnector(ConnectionID))
    {
        int32 QueryID = GenerateQueryID(ConnectionID);
        FAsyncTask<UpdateMySQLImageAsyncTask>* UpdateImageQueryTask = StartAsyncTask<UpdateMySQLImageAsyncTask>(
            this, CurrentConnector, ConnectionID, QueryID, Query, UpdateParameter, ParameterID, ImagePath);
        UpdateImageQueryTasks.Add(UpdateImageQueryTask);
    }
}

bool AMySQLDBConnectionActor::UpdateImageFromTexture(int32 ConnectionID, FString Query, FString UpdateParameter, 
                                                   int ParameterID, UTexture2D* Texture, EQueryExecutionContext ExecutionContext)
{
    // Special case - need to prepare image first
    FString ErrorMessage;
    if (!HandleQueryExecutionContext(ConnectionID, ExecutionContext, false, Query, ErrorMessage))
    {
        // Special case for image queries with texture
        if (ErrorMessage.IsEmpty() && !HasAuthority())
        {
            // Save texture locally first
            FString TexturePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("OutputImage.png"));
            TexturePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*TexturePath);
            
            if (Texture && UMySQLBPLibrary::SaveTextureToPath(Texture, TexturePath))
            {
                // Send request with path
                FQueryRequest Request;
                Request.ConnectionID = ConnectionID;
                Request.QueryString = Query;
                Request.UpdateParameter = UpdateParameter;
                Request.ParameterID = ParameterID;
                Request.ImagePath = TexturePath;
                Request.bIsSelectQuery = false;
                Request.bIsImageQuery = true;
                
                ServerExecuteImageQuery(Request);
                return true;
            }
            return false;
        }
        else if (!ErrorMessage.IsEmpty())
        {
            OnImageUpdateStatusChanged(ConnectionID, -1, false, ErrorMessage);
        }
        return false;
    }
    
    // Execute locally
    if (Texture)
    {
        FString TexturePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("OutputImage.png"));
        TexturePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*TexturePath);

        if (UMySQLBPLibrary::SaveTextureToPath(Texture, TexturePath))
        {
            UpdateImageFromPath(ConnectionID, Query, UpdateParameter, ParameterID, TexturePath, ExecutionContext);
            return true;
        }
    }
    
    return false;
}