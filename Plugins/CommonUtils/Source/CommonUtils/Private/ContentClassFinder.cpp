#include "ContentClassFinder.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"          // <-- defines FAssetData
#include "Engine/Blueprint.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/Engine.h"                    // <-- defines GEngine + GetWorldFromContextObjectChecked
#include "Engine/World.h"
#include "GameFramework/Actor.h"

static bool NamesEqual(const FString& A, const FString& B, bool bCaseSensitive)
{
    return bCaseSensitive ? A.Equals(B, ESearchCase::CaseSensitive)
                          : A.Equals(B, ESearchCase::IgnoreCase);
}

UClass* UContentClassFinder::FindClassByShortName(const FString& ShortClassName, TSubclassOf<UObject> BaseClass, bool bCaseSensitive)
{
    if (ShortClassName.IsEmpty())
    {
        return nullptr;
    }

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AR = AssetRegistryModule.Get();
    AR.WaitForCompletion();

    // Scan /Game for Blueprint assets
    TArray<FAssetData> Assets;
    FARFilter Filter;
    Filter.PackagePaths.Add(FName(TEXT("/Game")));
    Filter.bRecursivePaths = true;
    // UE5+ API: use ClassPaths (avoids the deprecation warning)
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());

    AR.GetAssets(Filter, Assets);

    // Fast path: check GeneratedClass tag (no asset load)
    for (const FAssetData& AD : Assets)
    {
        FString GenClassPathStr;
        if (!AD.GetTagValue(FName(TEXT("GeneratedClass")), GenClassPathStr))
        {
            continue;
        }

        FSoftClassPath SCP(GenClassPathStr);
        const FString CandidateShortName = SCP.GetAssetName(); // "MyBP_C"

        if (!NamesEqual(CandidateShortName, ShortClassName, bCaseSensitive))
        {
            continue;
        }

        if (UClass* Found = SCP.TryLoadClass<UObject>())
        {
            if (!*BaseClass || Found->IsChildOf(BaseClass))
            {
                return Found;
            }
        }
    }

    // Fallback: load BP assets and check GeneratedClass->GetName()
    for (const FAssetData& AD : Assets)
    {
        if (const UBlueprint* BP = Cast<UBlueprint>(AD.GetAsset()))
        {
            if (UClass* GenCls = BP->GeneratedClass)
            {
                if (NamesEqual(GenCls->GetName(), ShortClassName, bCaseSensitive))
                {
                    if (!*BaseClass || GenCls->IsChildOf(BaseClass))
                    {
                        return GenCls;
                    }
                }
            }
        }
    }

    return nullptr;
}

AActor* UContentClassFinder::SpawnActorByClassShortName(UObject* WorldContextObject, const FString& ShortClassName, const FTransform& Transform, bool bCaseSensitive)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObjectChecked(WorldContextObject) : nullptr;
    if (!World)
    {
        return nullptr;
    }

    if (UClass* Cls = FindClassByShortName(ShortClassName, AActor::StaticClass(), bCaseSensitive))
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("FindClassByShortName : found"));
        FActorSpawnParameters Params;
        return World->SpawnActor<AActor>(Cls, Transform, Params);
    }
    return nullptr;
}

AActor* UContentClassFinder::SpawnActorByClassFullPath(
    UObject* WorldContextObject,
    const FString& ClassPath,
    const FTransform& Transform,
    ESpawnActorCollisionHandlingMethod CollisionHandlingOverride)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("[CommonUtils] SpawnActorByClassFullPath: WorldContextObject is null"));
        return nullptr;
    }

    UWorld* World = GEngine ? GEngine->GetWorldFromContextObjectChecked(WorldContextObject) : nullptr;
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[CommonUtils] SpawnActorByClassFullPath: World is null"));
        return nullptr;
    }

    FSoftClassPath SCP(ClassPath);                 // accepts "/Game/..._C" or "Class'/Game/..._C'"
    UClass* Cls = SCP.TryLoadClass<AActor>();      // load the class
    if (!Cls)
    {
        UE_LOG(LogTemp, Error, TEXT("[CommonUtils] Could not load class from '%s'"), *ClassPath);
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = CollisionHandlingOverride;

    return World->SpawnActor<AActor>(Cls, Transform, Params);
}


// --- Live Coding link quirk workaround ---
// UE 5.6 defines UE::Math::TIntVector3<int>::ZeroValue, but LC patch
// sometimes can’t find it unless we force a reference (≤5.5 needs a def).
#if WITH_LIVE_CODING
#include "Math/IntVector.h"
#include "Runtime/Launch/Resources/Version.h"

// UE 5.6+ — engine defines ZeroValue; just force a reference
#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
static const volatile void* GForceLink_IntVectorZero_CommonUtils =
    &UE::Math::TIntVector3<int>::ZeroValue;
// UE ≤ 5.5 — provide a definition
#else
namespace UE { namespace Math {
    template<> const TIntVector3<int> TIntVector3<int>::ZeroValue(0, 0, 0);
}}
#endif
#endif