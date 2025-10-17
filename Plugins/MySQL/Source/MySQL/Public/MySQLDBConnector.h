// Copyright Athian Games. All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "MySQLBPLibrary.h"
#include "MySQLMain.h"
#include "MySQLDBConnector.generated.h"


UCLASS()
class MYSQL_API UMySQLDBConnector : public UObject
{
	GENERATED_BODY()

	UMySQLDBConnector();

	TMap<int, int> QueryToConnectionMap;
	
	
public:

	MySQLConnection* mysqlConnection;
	
	bool CreateNewConnection(int32 ConnectionID, FString Server, FString DBName, FString UserID, FString Password, int32 Port, TArray<FMySQLOptionPair> Options, 
	                         FString& ErrorMessage);

	
	void  CloseConnection(int32 ConnectionID);

	void UpdateDataFromQuery(int32 ConnectionID, int32 QueryID, FString Query, bool& IsSuccessful, FString& ErrorMessage);

	void SelectDataFromQuery(int32 ConnectionID, FString Query, bool& IsSuccessful, FString& ErrorMessage,
	                         TArray<FMySQLDataTable>& ResultByColumn, TArray<FMySQLDataRow>& ResultByRow);


	void UpdateImageFromPath(int32 ConnectionID, int32 QueryID, FString Query, FString UpdateParameter, int ParameterID, FString ImagePath, bool&
	                         IsSuccessful, FString& ErrorMessage);
	UTexture2D* SelectImageFromQuery(int32 ConnectionID, int32 QueryID, FString Query, bool& IsSuccessful, FString& ErrorMessage);


	virtual void BeginDestroy() override;
	
};

