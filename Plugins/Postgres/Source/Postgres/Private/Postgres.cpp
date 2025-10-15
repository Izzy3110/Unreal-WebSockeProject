// Must be first for explicit PCH
#include "Postgres.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Modules/ModuleManager.h"

static TArray<void*> GPostgresLoadedDlls;

static bool TryLoadFromDirs(const TArray<FString>& Dirs, const TCHAR* Name)
{
	for (const FString& Dir : Dirs)
	{
		const FString Full = FPaths::Combine(Dir, Name);
		if (FPaths::FileExists(Full))
		{
			if (void* H = FPlatformProcess::GetDllHandle(*Full))
			{
				GPostgresLoadedDlls.Add(H);
				return true;
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("[Postgres] Failed to load %s from any known dir"), Name);
	return false;
}

void FPostgresModule::StartupModule()
{
#if PLATFORM_WINDOWS
	const FString PluginBin = FPaths::GetPath(FModuleManager::Get().GetModuleFilename(TEXT("Postgres")));
	const FString ProjBin   = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64"));
	const FString EngBin    = FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/Win64"));
	const FString DevBin    = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectDir(), TEXT("Plugins/Postgres/ThirdParty/PostgreSQL/bin/Win64")));

	TArray<FString> Dirs = { PluginBin, ProjBin, EngBin, DevBin };

	// Load deps first, then libpq
	TryLoadFromDirs(Dirs, TEXT("libcrypto-3-x64.dll"));
	TryLoadFromDirs(Dirs, TEXT("libssl-3-x64.dll"));
	// Optional, if your libpq needs them:
	TryLoadFromDirs(Dirs, TEXT("libiconv-2.dll"));
	TryLoadFromDirs(Dirs, TEXT("libintl-9.dll"));
	TryLoadFromDirs(Dirs, TEXT("zlib1.dll"));
	// Finally libpq
	TryLoadFromDirs(Dirs, TEXT("libpq.dll"));
#endif
}

void FPostgresModule::ShutdownModule()
{
#if PLATFORM_WINDOWS
	for (void* H : GPostgresLoadedDlls)
	{
		FPlatformProcess::FreeDllHandle(H);
	}
	GPostgresLoadedDlls.Empty();
#endif
}

IMPLEMENT_MODULE(FPostgresModule, Postgres)

// Live Coding note (leave this block as you had it)
#if WITH_LIVE_CODING
	#include "Math/IntVector.h"
	#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
		static const volatile void* GForceLink_IntVectorZero =
			&UE::Math::TIntVector3<int>::ZeroValue;
	#else
		namespace UE { namespace Math {
			template<> const TIntVector3<int> TIntVector3<int>::ZeroValue(0, 0, 0);
		}}
	#endif
#endif
