// Must be first for explicit PCH
#include "Postgres.h"
#include "Runtime/Launch/Resources/Version.h"

#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Modules/ModuleManager.h"
#include "Misc/ScopeLock.h"

// ---------- Module implementation (do NOT re-declare the class) ----------
void FPostgresModule::StartupModule() {}
void FPostgresModule::ShutdownModule() {}
IMPLEMENT_MODULE(FPostgresModule, Postgres)

// ---------- Robust DLL loader (Windows) ----------
#if PLATFORM_WINDOWS
static FCriticalSection GPostgresLoaderMutex;
static TArray<void*>    GPostgresLoadedDlls;
static bool             GPostgresLibsReady   = false;
static bool             GPostgresTriedLoad   = false;

static bool TryLoadOne(const FString& Dir, const TCHAR* Name)
{
    const FString Full = FPaths::Combine(Dir, Name);
    if (!FPaths::FileExists(Full)) return false;
    if (void* H = FPlatformProcess::GetDllHandle(*Full))
    {
        GPostgresLoadedDlls.Add(H);
        return true;
    }
    return false;
}

static bool TryLoadFromDirs(const TArray<FString>& Dirs, const TCHAR* Name)
{
    for (const FString& D : Dirs)
    {
        if (TryLoadOne(D, Name))
        {
            return true;
        }
    }
    UE_LOG(LogTemp, Error, TEXT("[Postgres] Failed to load %s from known dirs."), Name);
    return false;
}

static void GetSearchDirs(TArray<FString>& Out)
{
    Out.Reset();

    // Plugin bin dir (…/Plugins/Postgres/Binaries/Win64)
    const FString ModulePath = FModuleManager::Get().GetModuleFilename(TEXT("Postgres"));
    Out.Add(FPaths::GetPath(ModulePath));

    // Project binaries
    Out.Add(FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64")));

    // Engine binaries (UE may stage there depending on config)
    Out.Add(FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/Win64")));

    // ThirdParty bin inside the plugin (dev convenience)
    Out.Add(FPaths::ConvertRelativePathToFull(
        FPaths::Combine(FPaths::ProjectDir(), TEXT("Plugins/Postgres/ThirdParty/PostgreSQL/bin/Win64"))));
}
#endif // PLATFORM_WINDOWS

bool Postgres_EnsureLibpqLoaded()
{
#if !PLATFORM_WINDOWS
    return true;
#else
    if (GPostgresLibsReady) return true;

    // If we've already tried and failed, bail fast.
    if (GPostgresTriedLoad) return false;

    FScopeLock Lock(&GPostgresLoaderMutex);
    if (GPostgresLibsReady) return true;

    TArray<FString> Dirs; GetSearchDirs(Dirs);

    // Try both OpenSSL 3 and 1.1 naming (depends on where libpq came from)
    const TCHAR* CryptoCandidates[] = { TEXT("libcrypto-3-x64.dll"), TEXT("libcrypto-1_1-x64.dll") };
    const TCHAR* SslCandidates[]    = { TEXT("libssl-3-x64.dll"),    TEXT("libssl-1_1-x64.dll")   };

    // Optional deps (loaded only if present)
    const TCHAR* OptionalDeps[] = {
        TEXT("libiconv-2.dll"),
        TEXT("libintl-9.dll"),
        TEXT("zlib1.dll"),
        TEXT("libzstd.dll"),
        TEXT("liblz4.dll"),
        TEXT("libwinpthread-1.dll"), // <-- add
    };

    // Load crypto/ssl (whichever variant exists)
    for (auto* N : CryptoCandidates) { if (TryLoadFromDirs(Dirs, N)) break; }
    for (auto* N : SslCandidates)    { if (TryLoadFromDirs(Dirs, N)) break; }

    // Load optional deps if available
    for (auto* N : OptionalDeps) { TryLoadFromDirs(Dirs, N); }

    // Must have libpq.dll
    if (!TryLoadFromDirs(Dirs, TEXT("libpq.dll")))
    {
        GPostgresTriedLoad = true;
        return false;
    }

    GPostgresLibsReady = true;
    return true;
#endif
}

// --- Live Coding note (keep as-is) ---
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
