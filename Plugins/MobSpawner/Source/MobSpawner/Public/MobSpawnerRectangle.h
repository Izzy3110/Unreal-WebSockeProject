#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MobSpawnerRectangle.generated.h"

class USplineComponent;

UCLASS()
class MOBSPAWNER_API AMobSpawnerRectangle : public AActor
{
	GENERATED_BODY()

public:
	AMobSpawnerRectangle();
	virtual ~AMobSpawnerRectangle() override;
	static constexpr bool bForceSpawnIgnoreCollisionsDefault = false;

	/** Reference point that actors should face */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner|Target")
	UStaticMeshComponent* TargetSphere;

	// Material property for TargetSphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Target")
	UMaterialInterface* TargetSphereMaterial;

	
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void RandomizeSamplePoints();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void RandomizeAndSpawnActors();
	
	// Rotates all spawned actors to face TargetSphereActor
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void UpdateSpawnedActorsRotation();

	/** Rectangle size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	FVector2D Size = FVector2D(1000.f, 500.f);

	/** Spline component representing rectangle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* Spline;

	/** Which Pawn classes can be spawned */
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

	/** Sample points (exposed so you can inspect in editor) */
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

	/** Checks if a point is inside the rectangle */
	UFUNCTION(BlueprintCallable, Category="Spawner")
	bool IsPointInsideSpline(const FVector& Point) const;

	/** Returns a random point inside the rectangle */
	UFUNCTION(BlueprintCallable, Category="Spawner")
	FVector GetRandomPointInsideSpline() const;

	/** Convert spline to 2D polygon by sampling positions along spline */
	UFUNCTION(BlueprintCallable, Category="Spawner")
	TArray<FVector2D> BuildSplinePolygon2D(int32 NumSamples) const;

	/** Returns sampled points along or inside the spline */
	UFUNCTION(BlueprintCallable, Category="Spawner")
	TArray<FVector> CalculateSamplePoints(bool bPerformOverlapTest = true) const;
	
	/** Whether to perform overlap collision checks when generating sample points.
	Turn off for editor preview (OnConstruction) if overlap tests give incorrect results in editor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor")
	bool bPerformCollisionCheck = true;
	
	// debug
	UFUNCTION(BlueprintCallable, Category="Spawner|Debug")
	void DrawDebugRectangle(float LifeTime) const;

	UFUNCTION(BlueprintCallable, Category="Spawner|Debug")
	void DrawDebugAtSamplePoints(float LifeTime) const;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Debug")
	void DrawPersistentDebugRectangle() const;

	/** Draw persistent debug spheres at sample points (LifeTime = -1) */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Debug")
	void DrawPersistentDebugSpheres();

	/** Remove all persistent debug */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Debug")
	void ClearPersistentDebug() const;

	// spawning
	/** Spawn actors at sample points (round-robin through SpawnablePawns). CallInEditor possible. */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void SpawnActorsFromSamples();

	/** Remove and destroy all actors that were spawned by SpawnActorsFromSamples */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Controls|Runtime")
	void RemoveSpawnedActors();

	// ---------------------------
	// Debug options
	// ---------------------------

	/** Whether to draw debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	bool bDrawDebug = true;

	/** Whether to update debug every tick (runtime only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	bool bDrawDebugEveryTick = false;

	/** Debug color for lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	FColor DebugColor_Lines = FColor::Green;

	/** Debug color for sample points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	FColor DebugColor = FColor::Red;

	/** Debug sphere radius for points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	float DebugPointSize = 10.f;

	/** If true, editor preview draws persistent debug spheres (LifeTime = -1). Use ClearPersistentDebug() to remove them. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	bool bPersistentPreview = false;

	/** Height offset above actor location to spawn pawns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Actor")
	float SpawnHeightOffset = 50.f;

	UFUNCTION(BlueprintCallable, Category="Spawner|Actor")
	bool GetGroundZAtLocation(const FVector& Location, float& OutZ) const;

	/** Whether the spline should be rebuilt procedurally from Size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Spline")
	bool bProceduralSpline = true;
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

private:
	/** Test if a world location is free for spawning (sphere overlap) */
	bool IsLocationFree(const FVector& WorldLocation, float Radius) const;
};
