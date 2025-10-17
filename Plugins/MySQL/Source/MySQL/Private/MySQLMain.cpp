// Copyright Athian Games. All Rights Reserved. 

#include "MySQLMain.h"

#define WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <windows.h>
#include <sstream> 
#include <string>
#include <vector>
#include <tchar.h>
#include <codecvt>
#include <map>
#include <mutex>

std::wstring s2ws(const std::string& str) {
	int slength = static_cast<int>(str.length()) + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
	std::vector<wchar_t> buf(len);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, &buf[0], len);
	return basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t>>(buf.begin(), buf.end());
}

void MySQLConnection::CloseConnection(int ConnectionID)
{
	if (ConnectionID < DBConnections.size())
	{
		if (MYSQL* CurrentDBConnection = DBConnections[ConnectionID])
		{
			mysql_close(CurrentDBConnection);  // Close the connection
			DBConnections[ConnectionID] = nullptr;  // Nullify the pointer in the vector for safety
		}
	}
}

void MySQLConnection::CloseAllConnections()
{
	for (int iIndex = 0; iIndex < DBConnections.size(); iIndex++)
	{
		CloseConnection(iIndex);
	}
}

MYSQL* MySQLConnection::GetDBConnection(int ConnectionID)
{
	if (ConnectionID < DBConnections.size())
	{
		if (MYSQL* CurrentDBConnection = DBConnections[ConnectionID])
		{
			// Check if the connection is down (mysql_ping returns non-zero when the connection is down)
			if (mysql_ping(CurrentDBConnection) != 0)
			{
				// Try to reconnect (using mysql_ping as an example, replace with mariadb_reconnect if that's valid)
				if (mysql_ping(CurrentDBConnection) != 0)  // Check again after trying to reconnect
					{
						return nullptr;  // If still down after reconnect attempt, return nullptr
					}
			}
			return CurrentDBConnection;
		}
	}
	return nullptr;
}


bool MySQLConnection::IsValidConnection(int ConnectionID)
{
	if (GetDBConnection(ConnectionID))
	{
		return true;
	}
	else
	{
		return false;
	}
}

unsigned int get_mysql_option(const std::string& key)
{
	// Convert the key to uppercase for case-insensitive comparison
	std::string upperKey;
	std::transform(key.begin(), key.end(), std::back_inserter(upperKey), ::toupper);

	// Mapping user-friendly strings to MariaDB options
	static const map<std::string, unsigned int> optionMap = {
		{"OPT_RECONNECT", MYSQL_OPT_RECONNECT},
		// Add other options here as required
	};

	auto it = optionMap.find(upperKey);
	if (it != optionMap.end())
		return it->second;

	return 0;  // indicates key not recognized or not supported
}

void MySQLConnection::SetMySQLBulkOptions(MYSQL* MySQLHandle, const TArray<FMySQLOptionPair>& OptionsArray)
{
    for (const FMySQLOptionPair& OptionPair : OptionsArray)
    {
        switch (OptionPair.Option)
        {
            // String Options
            case EMySQLOptions::INIT_COMMAND:
            case EMySQLOptions::READ_DEFAULT_FILE:
            case EMySQLOptions::READ_DEFAULT_GROUP:
            case EMySQLOptions::SET_CHARSET_DIR:
            case EMySQLOptions::SET_CHARSET_NAME:
            case EMySQLOptions::SHARED_MEMORY_BASE_NAME:
             case EMySQLOptions::DEFAULT_AUTH:
            case EMySQLOptions::OPT_SSL_KEY:
            case EMySQLOptions::OPT_SSL_CERT:
            case EMySQLOptions::OPT_SSL_CA:
            case EMySQLOptions::OPT_SSL_CAPATH:
            case EMySQLOptions::OPT_SSL_CIPHER:
            case EMySQLOptions::OPT_SSL_CRL:
            case EMySQLOptions::OPT_SSL_CRLPATH:
            case EMySQLOptions::OPT_TLS_PASSPHRASE:
                 SetMySQLOption(MySQLHandle, OptionPair.Option, OptionPair.Value);
                break;

            // Integer Options
            case EMySQLOptions::OPT_CONNECT_TIMEOUT:
            case EMySQLOptions::OPT_PROTOCOL:
            case EMySQLOptions::OPT_READ_TIMEOUT:
            case EMySQLOptions::OPT_WRITE_TIMEOUT:
            case EMySQLOptions::OPT_MAX_ALLOWED_PACKET:
            case EMySQLOptions::OPT_NET_BUFFER_LENGTH:
                SetMySQLOption(MySQLHandle, OptionPair.Option, FCString::Atoi(*OptionPair.Value));
                break;

            // Boolean Options
            case EMySQLOptions::OPT_COMPRESS:
            case EMySQLOptions::OPT_NAMED_PIPE:
            case EMySQLOptions::OPT_SSL_ENFORCE:
            case EMySQLOptions::OPT_CAN_HANDLE_EXPIRED_PASSWORDS:
            case EMySQLOptions::ENABLE_CLEARTEXT_PLUGIN:
                SetMySQLOption(MySQLHandle, OptionPair.Option, OptionPair.Value.Equals("true", ESearchCase::IgnoreCase));
                break;

            default:
                // Handle or log any unknown or unsupported options
                UE_LOG(LogTemp, Warning, TEXT("Unknown or unsupported MySQL option provided: %s"), *OptionPair.Value);
                break;
        }
    }
}


bool MySQLConnection::CreateConnection(int ConnectionID, char* Server, char* DBName, char* UserID, char* Password, int Port,  TArray<FMySQLOptionPair> Options, const char*& ErrorMessage)
{
    try
    {
        if (MYSQL* CurrentDBConnection = GetDBConnection(ConnectionID))
    	{
    		CloseConnection(ConnectionID);
    	}

    	MYSQL* CurrentDBConnection = mysql_init(nullptr);
    	if (!CurrentDBConnection)
    	{
    		ErrorMessage = "Failed to initialize MySQL connection.";
    		return false;
    	}

   
      	SetMySQLBulkOptions(CurrentDBConnection, Options);

      	mysql_ssl_set(CurrentDBConnection, NULL, NULL, NULL, NULL, NULL);

		if (!mysql_real_connect(CurrentDBConnection, Server, UserID, Password, DBName, Port, NULL, 0))
    	{
    		ErrorMessage = mysql_error(CurrentDBConnection);
    		mysql_close(CurrentDBConnection);
    		return false;
    	}
    	
    	if (DBConnections.size() <= ConnectionID)
    	{
    		DBConnections.push_back(CurrentDBConnection);
    	}
    	else
    	{
    		DBConnections[ConnectionID] = CurrentDBConnection;
    	}

    	return true;
    }
    catch (const std::exception& ex)
    {
        ErrorMessage = ex.what();
        CloseConnection(ConnectionID);
        return false;
    }
}

bool MySQLConnection::UpdateDataFromQuery(int ConnectionID, char* Query, const char*& ErrorMessage)
{
	if (MYSQL* CurrentDBConnection = GetDBConnection(ConnectionID))
	{
		try
		{
			MYSQL_STMT* stmt = mysql_stmt_init(CurrentDBConnection);
			if (!stmt)
			{
				ErrorMessage = "Failed to initialize statement";
				return false;
			}

			if (mysql_stmt_prepare(stmt, Query, strlen(Query)))
			{
				ErrorMessage = mysql_stmt_error(stmt);
				mysql_stmt_close(stmt);
				return false;
			}

			if (mysql_stmt_execute(stmt))
			{
				ErrorMessage = mysql_stmt_error(stmt);
				mysql_stmt_close(stmt);
				return false;
			}

			mysql_stmt_close(stmt);
			return true;
		}
		catch (const std::exception& ex)
		{
			ErrorMessage = ex.what();
		}
	}
	else
	{
		ErrorMessage = "Connection Not Found";
	}
	return false;
}

bool MySQLConnection::SelectDataFromQuery(int ConnectionID, const char* Query, std::vector<std::string>& ColumnNames,
	std::vector<std::vector<std::string>>& ColumnData, std::string& ErrorMessage)
{
	bool bStatus = false;
	MYSQL* CurrentDBConnection = GetDBConnection(ConnectionID);
	if (!CurrentDBConnection)
	{
		ErrorMessage = "Connection Not Found";
		return bStatus;
	}

	// Set character set to utf8mb4 before executing query
	mysql_set_character_set(CurrentDBConnection, "utf8mb4");

	if (mysql_query(CurrentDBConnection, Query))
	{
		ErrorMessage = mysql_error(CurrentDBConnection);
		return bStatus;
	}

	MYSQL_RES* result = mysql_store_result(CurrentDBConnection);
	if (!result)
	{
		ErrorMessage = "No result set returned from query.";
		return bStatus;
	}

	int num_fields = mysql_num_fields(result);
	MYSQL_FIELD* fields = mysql_fetch_fields(result);

	if (ColumnNames.empty())
	{
		for (int i = 0; i < num_fields; i++)
		{
			ColumnNames.push_back(fields[i].name);
		}
	}

	MYSQL_ROW row = mysql_fetch_row(result);
	unsigned long* lengths;
    
	while (row != nullptr)
	{
		std::vector<std::string> RowData;
		lengths = mysql_fetch_lengths(result);
        
		for (int i = 0; i < num_fields; i++)
		{
			if (row[i])
			{
				RowData.push_back(std::string(row[i], lengths[i]));
			}
			else
			{
				RowData.push_back("NULL");
			}
		}
		ColumnData.push_back(RowData);
		row = mysql_fetch_row(result);
	}

	mysql_free_result(result);
	bStatus = true;
	return bStatus;
}

bool MySQLConnection::UpdateImageFromPath(int ConnectionID, const char* Query, const char* ImageChar, const char*& ErrorMessage)
{
	MYSQL* CurrentDBConnection = GetDBConnection(ConnectionID);
	if (!CurrentDBConnection)
	{
		ErrorMessage = "Connection Not Found";
		return false;
	}

	MYSQL_STMT* stmt = mysql_stmt_init(CurrentDBConnection);
	if (!stmt)
	{
		ErrorMessage = "Failed to initialize statement.";
		return false;
	}

	if (mysql_stmt_prepare(stmt, Query, strlen(Query)))
	{
		ErrorMessage = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return false;
	}

	MYSQL_BIND bind;
	memset(&bind, 0, sizeof(bind));
	bind.buffer_type = MYSQL_TYPE_BLOB;
	bind.buffer = (void*)ImageChar;
	bind.buffer_length = strlen(ImageChar);

	if (mysql_stmt_bind_param(stmt, &bind))
	{
		ErrorMessage = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return false;
	}

	if (mysql_stmt_execute(stmt))
	{
		ErrorMessage = mysql_stmt_error(stmt);
		mysql_stmt_close(stmt);
		return false;
	}

	mysql_stmt_close(stmt);
	return true;
}

bool MySQLConnection::SelectImageFromQuery(int ConnectionID, const char* Query, char*& ImageChar, const char*& ErrorMessage)
{
	MYSQL* CurrentDBConnection = GetDBConnection(ConnectionID);
	if (!CurrentDBConnection)
	{
		ErrorMessage = "Connection Not Found";
		return false;
	}

	if (mysql_query(CurrentDBConnection, Query))
	{
		ErrorMessage = mysql_error(CurrentDBConnection);
		return false;
	}

	MYSQL_RES* res = mysql_store_result(CurrentDBConnection);
	if (res == nullptr)
	{
		ErrorMessage = mysql_error(CurrentDBConnection);
		return false;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if (row)
	{
		unsigned long* lengths = mysql_fetch_lengths(res);
		ImageChar = new char[lengths[0] + 1];
		memcpy(ImageChar, row[0], lengths[0]);
		ImageChar[lengths[0]] = 0;
	}
	else
	{
		ErrorMessage = "No data returned.";
		mysql_free_result(res);
		return false;
	}

	mysql_free_result(res);
	return true;
}
