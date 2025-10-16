#include "PGArrayParseLibrary.h"

static void StripBracesAndSpaces(FString& S)
{
	S.TrimStartAndEndInline();
	S.ReplaceInline(TEXT("{"), TEXT(""));
	S.ReplaceInline(TEXT("}"), TEXT(""));
	// optional: remove spaces
	S.ReplaceInline(TEXT(" "), TEXT(""));
}

bool UPGArrayParseLibrary::ParsePgArray(const FString& In, TArray<double>& OutNumbers, int32 ExpectedCount)
{
	OutNumbers.Reset();

	FString Clean = In;
	StripBracesAndSpaces(Clean);

	TArray<FString> Parts;
	Clean.ParseIntoArray(Parts, TEXT(","), /*CullEmpty*/ true);

	if (ExpectedCount >= 0 && Parts.Num() != ExpectedCount)
	{
		return false;
	}

	OutNumbers.Reserve(Parts.Num());
	for (const FString& P : Parts)
	{
		double Val = 0.0;
		if (!LexTryParseString(Val, *P))
		{
			return false;
		}
		OutNumbers.Add(Val);
	}
	return Parts.Num() > 0;
}

void UPGArrayParseLibrary::StringToVector3(const FString& In, bool& bSuccess, FVector& OutVector)
{
	TArray<double> Nums;
	bSuccess = ParsePgArray(In, Nums, /*ExpectedCount*/ 3);
	if (bSuccess)
	{
		OutVector.X = static_cast<float>(Nums[0]);
		OutVector.Y = static_cast<float>(Nums[1]);
		OutVector.Z = static_cast<float>(Nums[2]);
	}
	else
	{
		OutVector = FVector::ZeroVector;
	}
}

void UPGArrayParseLibrary::StringToRotator(const FString& In, bool& bSuccess, FRotator& OutRotator)
{
	TArray<double> Nums;
	bSuccess = ParsePgArray(In, Nums, /*ExpectedCount*/ 3);
	if (bSuccess)
	{
		OutRotator.Pitch = static_cast<float>(Nums[0]);
		OutRotator.Yaw   = static_cast<float>(Nums[1]);
		OutRotator.Roll  = static_cast<float>(Nums[2]);
	}
	else
	{
		OutRotator = FRotator::ZeroRotator;
	}
}

void UPGArrayParseLibrary::StringToFloatArray(const FString& In, bool& bSuccess, TArray<float>& OutValues)
{
	TArray<double> Nums;
	bSuccess = ParsePgArray(In, Nums, /*ExpectedCount*/ -1);
	OutValues.Reset();
	if (bSuccess)
	{
		OutValues.Reserve(Nums.Num());
		for (double D : Nums)
		{
			OutValues.Add(static_cast<float>(D));
		}
	}
}

void UPGArrayParseLibrary::StringToVector3Floats(const FString& In, bool& bSuccess, float& X, float& Y, float& Z)
{
	TArray<double> Nums;
	bSuccess = ParsePgArray(In, Nums, /*ExpectedCount*/ 3);
	if (bSuccess)
	{
		X = static_cast<float>(Nums[0]);
		Y = static_cast<float>(Nums[1]);
		Z = static_cast<float>(Nums[2]);
	}
	else
	{
		X = Y = Z = 0.f;
	}
}

FString UPGArrayParseLibrary::Vector3ToString(const FVector& V)
{
	// %g keeps it compact; adjust precision if you prefer
	return FString::Printf(TEXT("{%g,%g,%g}"), V.X, V.Y, V.Z);
}

FString UPGArrayParseLibrary::RotatorToString(const FRotator& R)
{
	return FString::Printf(TEXT("{%g,%g,%g}"), R.Pitch, R.Yaw, R.Roll);
}
