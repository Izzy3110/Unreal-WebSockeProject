// Copyright Athian Games. All Rights Reserved. 

#pragma once
#pragma comment(lib, "Pathcch.lib")
#include <vector>
#include <vector>
#include <xstring>
#include <mysql/mysql.h>

#include "MySQLConnectionOptions.h"

using namespace std;


class MySQLConnection
{


	void SetMySQLBulkOptions(MYSQL* MySQLHandle, const TArray<FMySQLOptionPair>& OptionsArray);
	template <typename T>
void SetMySQLOption(MYSQL* MySQLHandle, EMySQLOptions Option, const T& Value)
	{
		mysql_options(MySQLHandle, static_cast<enum mysql_option>(Option), &Value);
	}

public:

	vector<MYSQL*> DBConnections;
	MYSQL* GetDBConnection(int ConnectionID);

	MySQLConnection() = default;
	~MySQLConnection() = default;
    
	void CloseAllConnections();

	void CloseConnection(int ConnectionID);

	bool CreateConnection(int ConnectionID, char* Server, char* DBName, char* UserID, char* Password, int Port, TArray<FMySQLOptionPair> Options, const char*& ErrorMessage);
	bool UpdateDataFromQuery(int ConnectionID, char* Query, const char*& ErrorMessage);
	bool SelectDataFromQuery(int ConnectionID, const char* Query, std::vector<std::string>& ColumnNames, std::vector<std::vector<std::string>>&
	                         ColumnData, std::string& ErrorMessage);

	bool UpdateImageFromPath(int ConnectionID, const char* Query, const char* ImageChar, const char*& ErrorMessage);
	bool SelectImageFromQuery(int ConnectionID, const char* Query, char*& ImageChar, const char*& ErrorMessage);

	bool IsValidConnection(int ConnectionID);



};