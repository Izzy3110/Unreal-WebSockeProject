#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGArrayParseLibrary.generated.h"

/** Helpers to parse/format Postgres array literal strings like "{1,2,3}" */
UCLASS()
class COMMONUTILS_API UPGArrayParseLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Parse "{x,y,z}" into a Vector. Success=false if format is invalid. */
	UFUNCTION(BlueprintPure, Category="CommonUtils|Postgres")
	static void StringToVector3(const FString& In, bool& bSuccess, FVector& OutVector);

	/** Parse "{pitch,yaw,roll}" into a Rotator. Success=false if format is invalid. */
	UFUNCTION(BlueprintPure, Category="CommonUtils|Postgres")
	static void StringToRotator(const FString& In, bool& bSuccess, FRotator& OutRotator);

	/** Parse "{a,b,c,...}" into float array. Success=false if format is invalid. */
	UFUNCTION(BlueprintPure, Category="CommonUtils|Postgres")
	static void StringToFloatArray(const FString& In, bool& bSuccess, TArray<float>& OutValues);

	/** Convenience: split into three floats. */
	UFUNCTION(BlueprintPure, Category="CommonUtils|Postgres")
	static void StringToVector3Floats(const FString& In, bool& bSuccess, float& X, float& Y, float& Z);

	/** Format FVector as Postgres array: "{x,y,z}" */
	UFUNCTION(BlueprintPure, Category="CommonUtils|Postgres")
	static FString Vector3ToString(const FVector& V);

	/** Format FRotator as Postgres array: "{pitch,yaw,roll}" */
	UFUNCTION(BlueprintPure, Category="CommonUtils|Postgres")
	static FString RotatorToString(const FRotator& R);

private:
	static bool ParsePgArray(const FString& In, TArray<double>& OutNumbers, int32 ExpectedCount /* -1 = any count */);
};
