// Copyright Athian Games. All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MySQLBPLibrary.h"
#include "MySQLAsyncTasks.h"
#include "MySQLDBConnector.h"

#include "MySQLDBConnectionActor.generated.h"

UENUM()
enum EQueryType
{
    Update,
    Select,
    Close,
    Endplay
};

UENUM(BlueprintType)
enum class EMySQLReplicationMode : uint8
{
    ServerOnly UMETA(DisplayName = "Server Only - Only server can run queries"),
    ClientToServer UMETA(DisplayName = "Client To Server - Client can request server to run queries"),
    Multicast UMETA(DisplayName = "Multicast - Both client and server can run queries")
};

UENUM(BlueprintType)
enum class EQueryExecutionContext : uint8
{
    Default UMETA(DisplayName = "Default (Use Replication Mode)"),
    ForceServer UMETA(DisplayName = "Force Server Execution"),
    ForceClient UMETA(DisplayName = "Force Client Execution")
};

USTRUCT()
struct FQueryTaskData
{
    GENERATED_BODY()
    
    int32 ConnectionID;
    int32 QueryID;
    TArray<FString> Queries;
        
    EQueryType QueryType; // Define an enumeration EQueryType with values like Select, Update, etc.
    // Add any other required parameters for the query

    friend bool operator==(const FQueryTaskData& lhs, const FQueryTaskData& rhs)
    {
        return lhs.ConnectionID == rhs.ConnectionID &&  lhs.QueryID == rhs.QueryID;
    }
};

// Query request structure for client->server communication
USTRUCT()
struct FQueryRequest
{
    GENERATED_BODY()
    
    UPROPERTY()
    int32 ConnectionID;
    
    UPROPERTY()
    FString QueryString;
    
    UPROPERTY()
    bool bIsSelectQuery;
    
    // For image queries
    UPROPERTY()
    FString UpdateParameter;
    
    UPROPERTY()
    int32 ParameterID;
    
    UPROPERTY()
    FString ImagePath;
    
    UPROPERTY()
    bool bIsImageQuery;
};

// Structure to track client requests
USTRUCT()
struct FClientRequest
{
    GENERATED_BODY()
    
    UPROPERTY()
    int32 QueryID;
    
    UPROPERTY()
    AActor* RequestingClient;
};

UCLASS()
class MYSQL_API AMySQLDBConnectionActor : public AActor
{
    GENERATED_BODY()
    
    TMap<int32, int32> ConnectionToNextQueryIDMap;
    
   
template<typename TaskType, typename... Args>
FAsyncTask<TaskType>* StartAsyncTask(Args&&... args)
{
    TaskType Task(std::forward<Args>(args)...);
    FAsyncTask<TaskType>* AsyncTask = new FAsyncTask<TaskType>(std::move(Task));
    AsyncTask->StartBackgroundTask();
    return AsyncTask;
}

template<class T>
void CleanUpFinishedTasks(TArray<FAsyncTask<T>*> &TaskArray)
{
    for (int32 Index = 0; Index < TaskArray.Num(); ++Index)
    {
        FAsyncTask<T>* Task = TaskArray[Index];
        if (Task->IsDone())
        {
            delete Task;
            TaskArray.RemoveAt(Index--);
        }
    }
}
    
public:

    
    // Map to track which queries were requested by which clients
    TMap<int32, FClientRequest> ClientRequestMap;

    
    int32 GenerateQueryID(int32 ConnectionID);

    AMySQLDBConnectionActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UMySQLDBConnector* GetConnector(int32 ConnectionID);

    TArray<FAsyncTask<OpenMySQLConnectionTask>*> OpenConnectionTasks;
    TArray<FAsyncTask<UpdateMySQLQueryAsyncTask>*> UpdateQueryTasks;
    TArray<FAsyncTask<SelectMySQLQueryAsyncTask>*> SelectQueryTasks;
    TArray<FAsyncTask<UpdateMySQLImageAsyncTask>*> UpdateImageQueryTasks;
    TArray<FAsyncTask<SelectMySQLImageAsyncTask>*> SelectImageQueryTasks;
    
private:
    
    // Declare a queue to store the pending query tasks
    TArray<FQueryTaskData> QueryTaskQueue;

    // Declare a boolean to indicate whether a query task is currently running
    bool bIsQueryTaskRunning;
    void CreateTaskData(int32 ConnectionID, TArray<FString> Queries, EQueryType QueryType);

    UMySQLDBConnector* CreateDBConnector(int32& ConnectionID);

    static void CopyDLL(FString DLLName);

public:    

    // Method to initiate execution of next query in the queue
    void ExecuteNextQueryTask();
    void ResetLastConnection();

    UPROPERTY()
        bool bIsConnectionBusy;

    UPROPERTY()
        bool bIsSelectQueryBusy;

    UPROPERTY()
        TMap<int32, UMySQLDBConnector*> SQLConnectors;

    // Replication mode property
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication", ReplicatedUsing = OnRep_ReplicationModeChanged)
    EMySQLReplicationMode ReplicationMode;

    UFUNCTION()
    void OnRep_ReplicationModeChanged();

    // Override GetLifetimeReplicatedProps to set up property replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Helper function to check if the current execution context is valid for queries
    bool CanExecuteQueryInCurrentContext() const;
    
    UFUNCTION(Client, Reliable)
    void ClientReceiveConnectionStatus(APlayerController* Client, bool ConnectionStatus, int32 ConnectionID, const FString& ErrorMessage);


    // RPC for client to request server to execute a query
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerExecuteQuery(const FQueryRequest& QueryRequest);

    // RPC for client to request server to execute an image query
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerExecuteImageQuery(const FQueryRequest& QueryRequest);

    // Client-side callback functions (for receiving results from server)
    UFUNCTION(Client, Reliable)
    void ClientReceiveQueryResults(int32 ConnectionID, int32 QueryID, bool IsSuccessful, 
                                  const FString& ErrorMessage, const TArray<FMySQLDataTable>& ResultByColumn, 
                                  const TArray<FMySQLDataRow>& ResultByRow);

    UFUNCTION(Client, Reliable)
    void ClientReceiveUpdateStatus(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage);

    UFUNCTION(Client, Reliable)
    void ClientReceiveImageSelectStatus(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage, 
                                       UTexture2D* SelectedTexture);

    UFUNCTION(Client, Reliable)
    void ClientReceiveImageUpdateStatus(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage);

    FTimerHandle SelectDataTaskTimer;
    
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    
    /**
    * Creates a New Database Connection
    */
    UFUNCTION(BlueprintCallable, Category = "MySql Server")
        void CreateNewConnection(FString Server, FString DBName, FString UserID, FString Password, int32 Port);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MySQLOptions")
    UMySQLConnectionOptions* MySQLOptionsAsset;

    UFUNCTION(BlueprintCallable, Category = "MySql Server")
        void CloseAllConnections();

    UFUNCTION(BlueprintCallable, Category = "MySql Server")
        void CloseConnection(int32 ConnectionID);

    UFUNCTION(BlueprintImplementableEvent, Category = "MySql Server")
        void OnConnectionStateChanged(bool ConnectionStatus, int32 ConnectionID, const FString& ErrorMessage);

    UFUNCTION(BlueprintPure, Category = "MySql Server")
    int32 GetLastQueryID(int32 ConnectionID);

    UFUNCTION(BlueprintPure, Category = "MySql Server")
    bool CheckIsQueryRunning()
    {
        return bIsQueryTaskRunning;
    }

    bool HandleQueryExecutionContext(int32 ConnectionID, EQueryExecutionContext ExecutionContext, 
                                bool bIsSelectQuery, const FString& Query, 
                                FString& OutErrorMessage);



    /**
    * Executes a Query to the database
    */
    UFUNCTION(BlueprintCallable, Category = "MySql Server", meta=(AdvancedDisplay="ExecutionContext"))
    void UpdateDataFromQuery(int32 ConnectionID, FString Query, EQueryExecutionContext ExecutionContext = EQueryExecutionContext::Default);


    UFUNCTION(BlueprintImplementableEvent, Category = "MySql Server")
        void OnQueryUpdateStatusChanged(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage);


    /**
    * Selects data from the database
   */
    UFUNCTION(BlueprintCallable, Category = "MySql Server", meta=(AdvancedDisplay="ExecutionContext"))
    void SelectDataFromQuery(int32 ConnectionID, FString Query, EQueryExecutionContext ExecutionContext = EQueryExecutionContext::Default);
    

    UFUNCTION(BlueprintImplementableEvent, Category = "MySql Server")
        void OnQuerySelectStatusChanged(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage, const TArray<FMySQLDataTable>& ResultByColumn, 
            const TArray<FMySQLDataRow>& ResultByRow);


    UFUNCTION(BlueprintCallable, Category = "MySql Server", meta=(AdvancedDisplay="ExecutionContext"))
    void UpdateDataFromMultipleQueries(int32 ConnectionID, TArray<FString> Queries, EQueryExecutionContext ExecutionContext = EQueryExecutionContext::Default);

    UFUNCTION(BlueprintCallable, Category = "MySql Server", meta=(AdvancedDisplay="ExecutionContext"))
    void SelectImageFromQuery(int32 ConnectionID, FString Query, EQueryExecutionContext ExecutionContext = EQueryExecutionContext::Default);

    UFUNCTION(BlueprintCallable, Category = "MySql Server", meta=(AdvancedDisplay="ExecutionContext"))
    void UpdateImageFromPath(int32 ConnectionID, FString Query, FString UpdateParameter, int ParameterID, FString ImagePath, EQueryExecutionContext ExecutionContext = EQueryExecutionContext::Default);

    UFUNCTION(BlueprintCallable, Category = "MySql Server", meta=(AdvancedDisplay="ExecutionContext"))
    bool UpdateImageFromTexture(int32 ConnectionID, FString Query, FString UpdateParameter, int ParameterID, UTexture2D* Texture, EQueryExecutionContext ExecutionContext = EQueryExecutionContext::Default);

    
    UFUNCTION(BlueprintImplementableEvent, Category = "MySql Server")
        void OnImageUpdateStatusChanged(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage);

 
    UFUNCTION(BlueprintImplementableEvent, Category = "MySql Server")
        void OnImageSelectStatusChanged(int32 ConnectionID, int32 QueryID, bool IsSuccessful, const FString& ErrorMessage, UTexture2D* SelectedTexture);

    
};