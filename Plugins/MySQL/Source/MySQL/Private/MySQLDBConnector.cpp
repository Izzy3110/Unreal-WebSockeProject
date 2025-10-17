// Copyright Athian Games. All Rights Reserved. 


#include "MySQLDBConnector.h"

UMySQLDBConnector::UMySQLDBConnector()
{
	
}

void UMySQLDBConnector::BeginDestroy()
{
	if(mysqlConnection)
	{
		mysqlConnection->CloseAllConnections();
	}
	UObject::BeginDestroy();
}


bool UMySQLDBConnector::CreateNewConnection(int32 ConnectionID, FString Server, FString DBName, FString UserID, FString Password, int32 Port, TArray<FMySQLOptionPair> Options, FString& ErrorMessage)
{
	if(!mysqlConnection)
	{
		mysqlConnection = new  MySQLConnection();
	}

	string serverstring(TCHAR_TO_UTF8(*Server));
	char* server = _strdup(serverstring.c_str());

	string dbnamestring(TCHAR_TO_UTF8(*DBName));
	char* dbname = _strdup(dbnamestring.c_str());

	string useridstring(TCHAR_TO_UTF8(*UserID));
	char* userid = _strdup(useridstring.c_str());

	string passwordstring(TCHAR_TO_UTF8(*Password));
	char* password = _strdup(passwordstring.c_str());

	string Eparamstring;

	const char* errormessage = "";
	
	bool isConnectionSet = mysqlConnection->CreateConnection(ConnectionID, server, dbname, userid, password, Port, Options, errormessage);
	ErrorMessage = FString(UTF8_TO_TCHAR(errormessage));
	return isConnectionSet;


}


void UMySQLDBConnector::CloseConnection(int32 ConnectionID)
{
	if(IsValidLowLevel())
	{
		if(mysqlConnection)
		{
			mysqlConnection->CloseConnection(ConnectionID);
		}
	}
	
}

void UMySQLDBConnector::UpdateDataFromQuery(int32 ConnectionID, int32 QueryID, FString Query, bool& IsSuccessful, FString& ErrorMessage)
{
	IsSuccessful= false;

	if(IsValidLowLevel())
	{
		if(mysqlConnection && mysqlConnection->IsValidConnection(ConnectionID))
		{
		
			char* query = UMySQLBPLibrary::GetCharfromFString(Query);
			const char* errormessage = "";
	
			if (mysqlConnection->UpdateDataFromQuery(ConnectionID, query, errormessage))
			{
				IsSuccessful = true;
				// Save the query ID and associated connection ID
				QueryToConnectionMap.Add(QueryID, ConnectionID);
			}
			else
			{
				ErrorMessage = FString(UTF8_TO_TCHAR(errormessage));
			}
	
			ErrorMessage = FString(UTF8_TO_TCHAR(errormessage));
		
		}
		else
		{
			ErrorMessage = "Connection not Valid";
		}
	}
}


void UMySQLDBConnector::SelectDataFromQuery(int32 ConnectionID, FString Query, bool& IsSuccessful, FString& ErrorMessage,
	TArray<FMySQLDataTable>& ResultByColumn, TArray<FMySQLDataRow>& ResultByRow)
{
	IsSuccessful = false;

	if (mysqlConnection && mysqlConnection->IsValidConnection(ConnectionID))
	{
		std::string query(TCHAR_TO_UTF8(*Query));
		std::vector<std::string> ColumnNames;
		std::vector<std::vector<std::string>> ColumnData;
		std::string error;

		if (mysqlConnection->SelectDataFromQuery(ConnectionID, query.c_str(), ColumnNames, ColumnData, error))
		{
			IsSuccessful = true;

			for (const auto& ColumnName : ColumnNames)
			{
				FMySQLDataTable NewDataTable;
				NewDataTable.ColumnName = FString(ColumnName.c_str());
				ResultByColumn.Add(NewDataTable);
			}

			for (const auto& rowdata : ColumnData)
			{
				FMySQLDataRow Row;

				for (const auto& cdata : rowdata)
				{
					int32 CIndex = &cdata - &rowdata[0];
					if (ResultByColumn.Num() > CIndex)
					{
						ResultByColumn[CIndex].ColumnData.Add(FString(cdata.c_str()));
					}
					Row.RowData.Add(FString(cdata.c_str()));
				}

				ResultByRow.Add(Row);
			}
		}
		else
		{
			ErrorMessage = FString(error.c_str());
		}
	}
	else
	{
		ErrorMessage = "Connection not Valid";
	}

}

void UMySQLDBConnector::UpdateImageFromPath(int32 ConnectionID, int32 QueryID, FString Query,
	FString UpdateParameter, int ParameterID, FString ImagePath, bool& IsSuccessful, FString& ErrorMessage)
{

	IsSuccessful = false;
	if(mysqlConnection)
	{
		string query(TCHAR_TO_UTF8(*Query));
		char* querychar = _strdup(query.c_str());

		char* ImageChar = UMySQLBPLibrary::GetRawImageFromPath(ImagePath);

		const char* errormessage = "";

		if (mysqlConnection->UpdateImageFromPath(ConnectionID, querychar, ImageChar, errormessage))
		{
			IsSuccessful = true;
			
			// Save the query ID and associated connection ID
			QueryToConnectionMap.Add(QueryID, ConnectionID);
		}
		else
		{
			ErrorMessage = FString(UTF8_TO_TCHAR(errormessage));
		}
		
		
	}
	else
	{
		ErrorMessage = "Connection not Valid";
	}
	
}

UTexture2D* UMySQLDBConnector::SelectImageFromQuery(int32 ConnectionID, int32 QueryID, FString Query,
	bool& IsSuccessful, FString& ErrorMessage)
{

	UTexture2D* ImageTexture = nullptr;
	IsSuccessful = false;

	if(mysqlConnection)
	{
		string query(TCHAR_TO_UTF8(*Query));
		char* querychar = _strdup(query.c_str());

		char* ichar = nullptr;
		const char* errormessage = "";
		if(mysqlConnection->SelectImageFromQuery(ConnectionID, querychar, ichar,
		                                         errormessage))
		{
			IsSuccessful = true;
			// Save the query ID and associated connection ID
			QueryToConnectionMap.Add(QueryID, ConnectionID);
			
			char* ImageChar = const_cast<char*>(ichar);
			ImageTexture = UMySQLBPLibrary::LoadTexturefromCharData(ImageChar);
		}
		else
		{
			ErrorMessage = FString(UTF8_TO_TCHAR(errormessage));
		}
	}
	else
	{
		ErrorMessage = "Connection not Valid";
	}
	
	return ImageTexture;

}

