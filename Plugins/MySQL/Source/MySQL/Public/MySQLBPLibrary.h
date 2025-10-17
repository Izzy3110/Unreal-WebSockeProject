// Copyright Athian Games. All Rights Reserved. 

#pragma once


#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Modules/ModuleManager.h"
#include "MySQLBPLibrary.generated.h"


USTRUCT(BlueprintType, Category = "MySql|Tables")
struct FMySQLDataTable
{
	GENERATED_BODY()
		
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SQLDataTable")
		FString ColumnName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SQLDataTable")
		TArray<FString> ColumnData;
};

USTRUCT(BlueprintType, Category = "MySql|Tables")
struct FMySQLDataRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SQLDataRow")
		TArray<FString> RowData;
};


/**
* Contains all the methods that are used to connect to the C# dll 
* which takes care of connecting to the MySQL server and executing
* the given queries. These methods can be directly called from GetErrorMessage
* the Blueprints.

*/
UCLASS()
class MYSQL_API UMySQLBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:

	static char* GetCharFromTextureData(UTexture2D *Texture, FString Path);
	static UTexture2D* LoadTexturefromCharData(char* ImageChar);
	
	static void CreateImageWrapperModule();
	static char* GetRawImageFromPath(FString ImagePath);

	static bool SaveTextureToPath(UTexture2D* Texture, const FString Path);

	static void GetTexturePixels(UTexture2D* Texture, TArray<FColor>& OutPixels);

	static wchar_t* GetWCharfromChar(const char* Input);
	static char* GetCharfromFString(FString Query);
	static TArray<FString> GetSplitStringArray(FString Input, FString Pattern);




};
