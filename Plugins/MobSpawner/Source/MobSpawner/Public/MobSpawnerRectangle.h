#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MobSpawnerRectangle.generated.h"

class USplineComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class APawn;

UCLASS()
class MOBSPAWNER_API AMobSpawnerRectangle : public AActor
{
	GENERATED_BODY()

public:
	AMobSpawnerRectangle();
	virtual ~AMobSpawnerRectangle() override;

	/** If true, when spline or properties change in the editor, we automatically remove and respawn actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Editor")
	bool bAutoRespawnOnEdit = true;

	/** Reference point that actors should face */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner|Target")
	UStaticMeshComponent* TargetSphere;

	/** Material property for TargetSphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Target")
	UMaterialInterface* TargetSphereMaterial;

#if WITH_EDITOR
	/** Guard to avoid re-entrancy when we modify spline/actors during the callback */
	bool bHandlingSplineUpdate = false;
#endif

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSplineUpdated);
	UPROPERTY(BlueprintAssignable, Category="Spline")
	FOnSplineUpdated OnSplineUpdated;

#if WITH_EDITOR
	/** Called when any UObject property is changed in the editor (we listen to this to detect spline edits). */
	void OnEditorObjectPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	/** Register editor delegates */
	void RegisterEditorDelegates();
	
	/** Unregister editor delegates */
	void UnregisterEditorDelegates();
#endif
		
	// Editor controls
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void RandomizeSamplePoints();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void RandomizeAndSpawnActors();
	
	// Rotates all spawned actors to face TargetSphere
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void UpdateSpawnedActorsRotation();

	/** Rectangle size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	FVector2D Size = FVector2D(1000.f, 500.f);

	/** Spline component representing rectangle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
	USplineComponent* Spline;

	/** Which Pawn classes can be spawned? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor")
	TArray<TSubclassOf<APawn>> SpawnablePawns;

	/** Whether the rectangle spline should be closed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Spline")
	bool bRequireClosedSpline = true;

	/** Maximum number of points to sample for spawning actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor")
	int32 MaxActorCount = 32;

	/** Distance (in world units) between sampled points along the spline */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	float SampleSpacing = 200.0f;

	/** Sample points (exposed so you can inspect in the editor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor")
	TArray<FVector> SamplePoints;

	/** Radius used for overlap test when checking spawn locations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor", meta=(ClampMin="0.0"))
	float SpawnCollisionRadius = 50.f;

	/** Collision channel to test for blocking overlaps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor")
	TEnumAsByte<ECollisionChannel> SpawnCollisionChannel = ECC_Pawn;

	/** Extra attempts when sampling interior points to try to find a free location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor", meta=(ClampMin="1"))
	int32 MaxSpawnAttemptsPerPoint = 8;

	/** Actors that were spawned by this spawner (transient, cleared by RemoveSpawnedActors) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Spawner|Runtime")
	TArray<AActor*> SpawnedActors;

	/** Whether to perform overlap collision checks when generating sample points.
	  Turn off for editor preview (OnConstruction) if overlap tests give incorrect results in the editor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor")
	bool bPerformCollisionCheck = true;

	// debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	bool bDrawDebugEveryTick = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	FColor DebugColor_Lines = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	FColor DebugColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	float DebugPointSize = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	bool bPersistentPreview = false;

	/** Height offset above trace impact to spawn pawns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor")
	float SpawnHeightOffset = 50.f;

	/** Whether the spline should be rebuilt procedurally from Size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Spline")
	bool bProceduralSpline = true;

	UFUNCTION(BlueprintCallable, Category="Spawner")
	TArray<FVector2D> BuildSplinePolygon2D(int32 NumSamples) const;

	UFUNCTION(BlueprintCallable, Category="Spawner")
	TArray<FVector> CalculateSamplePoints(bool bPerformOverlapTest = true) const;

	UFUNCTION(BlueprintCallable, Category="Spawner|Debug")
	void DrawDebugRectangle(float LifeTime) const;

	UFUNCTION(BlueprintCallable, Category="Spawner|Debug")
	void DrawDebugAtSamplePoints(float LifeTime) const;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Debug")
	void DrawPersistentDebugSpheres();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Debug")
	void ClearPersistentDebug() const;

	// spawning
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void SpawnActorsFromSamples();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void RemoveSpawnedActors();

	UFUNCTION(BlueprintCallable, Category="Spawner|Actor")
	bool GetGroundZAtLocation(const FVector& Location, float& OutZ) const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** Test if a world location is free for spawning (sphere overlap) */
	bool IsLocationFree(const FVector& WorldLocation, float Radius) const;

#if WITH_EDITOR
	/** Called after spline or relevant properties were edited in the editor. */
	void OnSplineEdited();
#endif
};
