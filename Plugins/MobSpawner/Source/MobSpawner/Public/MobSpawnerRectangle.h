// MobSpawnerRectangle.h
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

	/** Rectangle size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	FVector2D Size = FVector2D(1000.f, 500.f);

	/** Spline component representing rectangle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* Spline;

	/** Whether the rectangle spline should be closed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	bool ClosedLoopRequired = true;

	/** Checks if a point is inside the rectangle */
	UFUNCTION(BlueprintCallable, Category="Spawner")
	bool IsPointInsideSpline(const FVector& Point) const;

	/** Returns a random point inside the rectangle */
	UFUNCTION(BlueprintCallable, Category="Spawner")
	FVector GetRandomPointInsideSpline() const;

	// ---------------------------
	// Debug options
	// ---------------------------

	/** Whether to draw debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	bool bDrawDebug = true;

	/** Debug color for lines and points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	FColor DebugColor = FColor::Green;

	/** Debug sphere radius for points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	float DebugPointSize = 10.f;

	/** Draw debug rectangle and points in the editor or runtime */
	UFUNCTION(CallInEditor, BlueprintCallable, Category="Spawner|Debug")
	void DrawDebugRectangle() const;

	/** Whether to update debug every tick (runtime only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner|Debug")
	bool bDrawDebugEveryTick = false;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
};
