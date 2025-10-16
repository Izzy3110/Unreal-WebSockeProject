#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "Engine/EngineTypes.h" // <-- adds ESpawnActorCollisionHandlingMethod
#include "ContentClassFinder.generated.h"

UCLASS()
class COMMONUTILS_API UContentClassFinder : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Find a class by short name (e.g., "Foo_C"). Optionally restrict to subclasses of BaseClass. */
	UFUNCTION(BlueprintCallable, Category="Content|Classes")
	static UClass* FindClassByShortName(
		const FString& ShortClassName,
		TSubclassOf<UObject> BaseClass,     // OK to keep as-is
		bool bCaseSensitive = false);

	/** Spawn an Actor by its short class name at Transform. Returns the spawned actor or nullptr. */
	UFUNCTION(BlueprintCallable, Category="Content|Classes", meta=(WorldContext="WorldContextObject"))
	static AActor* SpawnActorByClassShortName(
		UObject* WorldContextObject,
		const FString& ShortClassName,
		const FTransform& Transform,
		bool bCaseSensitive = false);
	
	UFUNCTION(BlueprintCallable, Category="Content|Classes",
		meta=(WorldContext="WorldContextObject",
			  AdvancedDisplay="CollisionHandlingOverride",
			  CPP_Default_CollisionHandlingOverride="AdjustIfPossibleButAlwaysSpawn"))
	static AActor* SpawnActorByClassFullPath(
		UObject* WorldContextObject,
		const FString& ClassPath,
		const FTransform& Transform,
		ESpawnActorCollisionHandlingMethod CollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
};
