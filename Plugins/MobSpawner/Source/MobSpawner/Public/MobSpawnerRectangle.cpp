#include "MobSpawnerRectangle.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Pawn.h"

AMobSpawnerRectangle::AMobSpawnerRectangle()
{
	PrimaryActorTick.bCanEverTick = true;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;

	TargetSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetSphere"));
	TargetSphere->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		TargetSphere->SetStaticMesh(SphereMesh.Object);
		TargetSphere->SetWorldScale3D(FVector(0.5f)); // adjust size
		TargetSphere->SetMobility(EComponentMobility::Movable);
		TargetSphere->SetMaterial(0, nullptr); // optional: assign yellow material
		TargetSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AMobSpawnerRectangle::OnConstruction(const FTransform& Transform)
{
	if (!Spline) return;

	// Only rebuild if procedural mode is enabled
	if (bProceduralSpline || Spline->GetNumberOfSplinePoints() == 0)
	{
		Spline->ClearSplinePoints(false);
		const FVector Half = FVector(Size.X / 2.f, Size.Y / 2.f, 0.f);

		Spline->AddSplinePoint(FVector(-Half.X, -Half.Y, 0.f), ESplineCoordinateSpace::Local);
		Spline->AddSplinePoint(FVector(Half.X, -Half.Y, 0.f), ESplineCoordinateSpace::Local);
		Spline->AddSplinePoint(FVector(Half.X, Half.Y, 0.f), ESplineCoordinateSpace::Local);
		Spline->AddSplinePoint(FVector(-Half.X, Half.Y, 0.f), ESplineCoordinateSpace::Local);

		if (bRequireClosedSpline && !Spline->IsClosedLoop())
		{
			Spline->SetClosedLoop(true);
		}

		Spline->UpdateSpline();

		if (TargetSphere)
		{
			TargetSphere->SetWorldLocation(GetActorLocation() + FVector(0.f, 0.f, 200.f)); // height offset
			TargetSphere->SetMobility(EComponentMobility::Movable);
			TargetSphere->SetVisibility(true);
		}
	}

#if WITH_EDITOR
	SamplePoints = CalculateSamplePoints(false);
	if (bDrawDebug)
	{
		DrawDebugRectangle(0.05f);
		DrawDebugAtSamplePoints(0.05f);
	}

	// Only update rotations if actors exist
	if (SpawnedActors.Num() > 0)
	{
		UpdateSpawnedActorsRotation();
	}
#endif


}


void AMobSpawnerRectangle::BeginPlay()
{
	Super::BeginPlay();

	SamplePoints = CalculateSamplePoints(bPerformCollisionCheck);

	if (bDrawDebug)
	{
		DrawDebugRectangle(0.1f);
		DrawDebugAtSamplePoints(0.1f);
	}
}

void AMobSpawnerRectangle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Your debug drawing (optional)
	if (bDrawDebug && bDrawDebugEveryTick)
	{
		const float Life = DeltaTime * 1.1f;
		DrawDebugRectangle(Life);

		if (!bPersistentPreview)
			DrawDebugAtSamplePoints(Life);
	}

	// Update spawned actors to face TargetSphereActor
	if (TargetSphere && SpawnedActors.Num() > 0)
	{
		const FVector TargetLoc = TargetSphere->GetComponentLocation();

		for (AActor* Actor : SpawnedActors)
		{
			if (!IsValid(Actor)) continue;

			FVector Dir = TargetLoc - Actor->GetActorLocation();
			Dir.Z = 0; // keep actors upright (yaw only)
			if (!Dir.IsNearlyZero())
			{
				FRotator Rot = Dir.Rotation();
				Actor->SetActorRotation(Rot);
			}
		}
	}
}



bool AMobSpawnerRectangle::IsPointInsideSpline(const FVector& Point) const
{
	const FVector LocalPoint = GetActorTransform().InverseTransformPosition(Point);
	const float HalfX = Size.X / 2.f;
	const float HalfY = Size.Y / 2.f;

	const bool bInside = (LocalPoint.X >= -HalfX && LocalPoint.X <= HalfX) &&
						 (LocalPoint.Y >= -HalfY && LocalPoint.Y <= HalfY);

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), Point, DebugPointSize, 12, bInside ? FColor::Green : FColor::Red, false, 0.f);
	}

	return bInside;
}

// --- Helper: point-in-polygon (ray-casting) for 2D polygon ---
static bool IsPointInPolygon(const TArray<FVector2D>& Polygon, const FVector2D& Point)
{
	const int32 Num = Polygon.Num();
	if (Num < 3) return false;

	bool bInside = false;
	for (int i = 0, j = Num - 1; i < Num; j = i++)
	{
		const FVector2D& Pi = Polygon[i];
		const FVector2D& Pj = Polygon[j];
		if (((Pi.Y > Point.Y) != (Pj.Y > Point.Y)) &&
			(Point.X < (Pj.X - Pi.X) * (Point.Y - Pi.Y) / (Pj.Y - Pi.Y + SMALL_NUMBER) + Pi.X))
		{
			bInside = !bInside;
		}
	}
	return bInside;
}

TArray<FVector2D> AMobSpawnerRectangle::BuildSplinePolygon2D(int32 NumSamples) const
{
	TArray<FVector2D> Poly;
	if (!Spline || NumSamples <= 2) return Poly;

	const float SplineLength = Spline->GetSplineLength();
	if (SplineLength <= 0.f) return Poly;

	const float Spacing = SplineLength / NumSamples;
	for (int32 i = 0; i < NumSamples; ++i)
	{
		const float Distance = FMath::Clamp(i * Spacing, 0.f, SplineLength);
		const FVector WorldPos = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		Poly.Add(FVector2D(WorldPos.X, WorldPos.Y));
	}
	return Poly;
}

static bool FindRandomPointInPolygon(const TArray<FVector2D>& Poly, const FBox2D& BBox, FVector2D& OutPoint, int32 MaxAttempts = 200)
{
	if (Poly.Num() < 3) return false;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const FVector2D Candidate(FMath::FRandRange(BBox.Min.X, BBox.Max.X), FMath::FRandRange(BBox.Min.Y, BBox.Max.Y));
		if (IsPointInPolygon(Poly, Candidate))
		{
			OutPoint = Candidate;
			return true;
		}
	}
	return false;
}

static FBox2D ComputePolygonBBox(const TArray<FVector2D>& Poly)
{
	if (Poly.Num() == 0) return FBox2D(EForceInit::ForceInitToZero);

	FVector2D Min = Poly[0], Max = Poly[0];
	for (const FVector2D& P : Poly)
	{
		Min.X = FMath::Min(Min.X, P.X); Min.Y = FMath::Min(Min.Y, P.Y);
		Max.X = FMath::Max(Max.X, P.X); Max.Y = FMath::Max(Max.Y, P.Y);
	}
	return FBox2D(Min, Max);
}

bool AMobSpawnerRectangle::IsLocationFree(const FVector& WorldLocation, float Radius) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(IsLocationFree), false);
	Params.AddIgnoredActor(this);

	bool bFree = !World->OverlapAnyTestByChannel(
		WorldLocation, 
		FQuat::Identity, 
		SpawnCollisionChannel, 
		Shape, 
		Params
	);
	
	// If blocked only by small Z difference (floor), allow spawning
	if (!bFree)
	{
		FHitResult Hit;
		if (World->SweepSingleByChannel(Hit, WorldLocation, WorldLocation + FVector(0,0,1), FQuat::Identity, SpawnCollisionChannel, Shape, Params))
		{
			if (Hit.Normal.Z > 0.7f) // mostly floor
				bFree = true;
		}
	}

	return bFree;
}

TArray<FVector> AMobSpawnerRectangle::CalculateSamplePoints(bool bPerformOverlapTest) const
{
	TArray<FVector> OutPoints;
	if (!Spline) return OutPoints;

	const bool bCheck = bPerformOverlapTest && bPerformCollisionCheck;
	
	const float SplineLength = Spline->GetSplineLength();

	// --------------------------
	// 1) Interior polygon sampling
	// --------------------------
	if (bRequireClosedSpline && Spline->IsClosedLoop())
	{
		const int32 NumPolySamples = FMath::Max(4, FMath::FloorToInt(SplineLength / FMath::Max(1.f, SampleSpacing)));
		TArray<FVector2D> Poly2D = BuildSplinePolygon2D(NumPolySamples);

		if (Poly2D.Num() >= 3)
		{
			FBox2D BBox = ComputePolygonBBox(Poly2D);
			const int32 TargetPoints = FMath::Max(1, MaxActorCount);

			OutPoints.Reserve(TargetPoints);

			for (int32 i = 0; i < TargetPoints; ++i)
			{
				bool bFound = false;
				for (int32 Attempt = 0; Attempt < MaxSpawnAttemptsPerPoint; ++Attempt)
				{
					FVector2D Candidate2D;
					if (!FindRandomPointInPolygon(Poly2D, BBox, Candidate2D))
						break;

					// Corrected: use world coordinates directly
					float GroundZ = 0.f;
					if (!GetGroundZAtLocation(FVector(Candidate2D.X, Candidate2D.Y, GetActorLocation().Z), GroundZ))
					{
						GroundZ = GetActorLocation().Z + SpawnHeightOffset; // fallback
					}

					FVector CandidateWorld(Candidate2D.X, Candidate2D.Y, GroundZ);

					if (!bCheck || IsLocationFree(CandidateWorld, SpawnCollisionRadius))
					{
						OutPoints.Add(CandidateWorld);
						bFound = true;
						break;
					}
				}

				(void)bFound; // skip if failed
			}

			if (OutPoints.Num() > 0)
				return OutPoints;
		}
	}

	// --------------------------
	// 2) Fallback: sample along spline line
	// --------------------------
	if (SplineLength <= 0.f) return OutPoints;

	if (MaxActorCount > 0)
	{
		const int32 NumPoints = FMath::Max(1, MaxActorCount);
		OutPoints.Reserve(NumPoints);

		for (int32 i = 0; i < NumPoints; ++i)
		{
			const float T = (NumPoints == 1) ? 0.f : static_cast<float>(i) / (NumPoints - 1);
			const float Distance = FMath::Clamp(T * SplineLength, 0.f, SplineLength);
			const FVector WorldPos = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

			if (!bCheck || IsLocationFree(WorldPos, SpawnCollisionRadius))
				OutPoints.Add(WorldPos);
		}
	}
	else
	{
		const float Spacing = FMath::Max(1.f, SampleSpacing);
		const int32 NumSamples = FMath::FloorToInt(SplineLength / Spacing);

		for (int32 i = 0; i < NumSamples; ++i)
		{
			const float Distance = FMath::Clamp(i * Spacing, 0.f, SplineLength);
			const FVector WorldPos = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

			if (!bCheck || IsLocationFree(WorldPos, SpawnCollisionRadius))
				OutPoints.Add(WorldPos);
		}
	}

	if (OutPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: CalculateSamplePoints produced 0 points. SplineLength=%.2f, SampleSpacing=%.2f, bRequireClosedSpline=%d, bCheck=%d"),
			*GetNameSafe(this), SplineLength, SampleSpacing, (int)bRequireClosedSpline, (int)bCheck);
	}

	return OutPoints;
}

FVector AMobSpawnerRectangle::GetRandomPointInsideSpline() const
{
	const float HalfX = Size.X / 2.f;
	const float HalfY = Size.Y / 2.f;
	const FVector RandomPoint = GetActorTransform().TransformPosition(
		FVector(FMath::FRandRange(-HalfX, HalfX), FMath::FRandRange(-HalfY, HalfY), 0.f)
	);

	if (bDrawDebug)
		DrawDebugSphere(GetWorld(), RandomPoint, DebugPointSize, 12, DebugColor, false, 0.f);

	return RandomPoint;
}

void AMobSpawnerRectangle::DrawDebugRectangle(float LifeTime) const
{
	if (!Spline) return;

	const FVector Half = FVector(Size.X / 2.f, Size.Y / 2.f, 0.f);
	const FVector WorldPoints[4] = {
		GetActorTransform().TransformPosition(FVector(-Half.X, -Half.Y, 0.f)),
		GetActorTransform().TransformPosition(FVector(Half.X, -Half.Y, 0.f)),
		GetActorTransform().TransformPosition(FVector(Half.X, Half.Y, 0.f)),
		GetActorTransform().TransformPosition(FVector(-Half.X, Half.Y, 0.f))
	};

	for (int32 i = 0; i < 4; ++i)
	{
		DrawDebugLine(GetWorld(), WorldPoints[i], WorldPoints[(i + 1) % 4], DebugColor_Lines, false, LifeTime, 0, 2.f);
	}
}

void AMobSpawnerRectangle::DrawDebugAtSamplePoints(float LifeTime) const
{
	if (!Spline || SamplePoints.Num() == 0) return;

	if (bPersistentPreview)
	{
#if WITH_EDITOR
		ClearPersistentDebug();

		for (const FVector& Point : SamplePoints)
			DrawDebugSphere(GetWorld(), Point, DebugPointSize, 12, DebugColor, true, -1.f, 0, 2.f);
#endif
	}
	else
	{
		for (const FVector& Point : SamplePoints)
			DrawDebugSphere(GetWorld(), Point, DebugPointSize, 12, DebugColor, false, LifeTime, 0, 2.f);
	}
}

void AMobSpawnerRectangle::DrawPersistentDebugRectangle() const
{
	DrawDebugRectangle(-1.f);
}

void AMobSpawnerRectangle::DrawPersistentDebugSpheres()
{
#if WITH_EDITOR
	if (!Spline || SamplePoints.Num() == 0) return;

	for (const FVector& Point : SamplePoints)
		DrawDebugSphere(GetWorld(), Point, DebugPointSize, 12, DebugColor, true, -1.f, 0, 2.f);
#endif
}

void AMobSpawnerRectangle::ClearPersistentDebug() const
{
#if WITH_EDITOR
	if (GetWorld())
		FlushPersistentDebugLines(GetWorld());
#endif
}

void AMobSpawnerRectangle::SpawnActorsFromSamples()
{
	if (SpawnedActors.Num() > 0)
		RemoveSpawnedActors();

	ClearPersistentDebug();
	
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: SpawnActorsFromSamples: World is null! Cannot spawn."), *GetNameSafe(this));
		return;
	}

	if (SamplePoints.Num() == 0)
	{
		SamplePoints = CalculateSamplePoints(true);
		UE_LOG(LogTemp, Log, TEXT("%s: SpawnActorsFromSamples: found %d sample points (with collision checks)."), *GetNameSafe(this), SamplePoints.Num());
	}

	if (SamplePoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No sample points found with collision checks. Retrying without checks for debug..."), *GetNameSafe(this));
		SamplePoints = CalculateSamplePoints(false);
		UE_LOG(LogTemp, Log, TEXT("%s: SpawnActorsFromSamples: found %d sample points (without collision checks)."), *GetNameSafe(this), SamplePoints.Num());
	}

	if (SamplePoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: SpawnActorsFromSamples: Still no sample points. Check SampleSpacing / MaxActorCount / collision settings."), *GetNameSafe(this));
		return;
	}

	if (SpawnablePawns.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: SpawnActorsFromSamples: SpawnablePawns array is empty. Assign pawn BP classes in the Details panel."), *GetNameSafe(this));
		return;
	}

	int32 PawnIndex = 0;
	int32 SpawnedCount = 0;
	for (const FVector& Loc : SamplePoints)
	{
		if (!IsLocationFree(Loc, SpawnCollisionRadius))
		{
			UE_LOG(LogTemp, Verbose, TEXT("%s: SpawnActorsFromSamples: location blocked at (%.1f, %.1f, %.1f), skipping."), *GetNameSafe(this), Loc.X, Loc.Y, Loc.Z);
			continue;
		}

		TSubclassOf<APawn> PawnClass = SpawnablePawns[PawnIndex % SpawnablePawns.Num()];
		++PawnIndex;

		if (!PawnClass) continue;

		FActorSpawnParameters Params;
		Params.Owner = this;
#if WITH_EDITOR
		Params.ObjectFlags = RF_Transactional;
#endif
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (APawn* Spawned = GetWorld()->SpawnActor<APawn>(PawnClass, Loc, FRotator::ZeroRotator, Params))
		{
			SpawnedActors.Add(Spawned);

			// Make actor face the sphere
			if (TargetSphere)
			{
				FVector Direction = TargetSphere->GetComponentLocation() - Spawned->GetActorLocation();
				Direction.Z = 0; // keep upright
				if (!Direction.IsNearlyZero())
				{
					FRotator NewRot = Direction.Rotation();
					Spawned->SetActorRotation(NewRot);
				}
			}
			
			++SpawnedCount;
#if WITH_EDITOR
			Spawned->SetFlags(RF_Transactional);
#endif
			UE_LOG(LogTemp, Log, TEXT("%s: SpawnActorsFromSamples: Spawned '%s' at (%.1f, %.1f, %.1f)"), *GetNameSafe(this), *Spawned->GetName(), Loc.X, Loc.Y, Loc.Z);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: SpawnActorsFromSamples: Spawn failed for class %s at (%.1f, %.1f, %.1f)"),
				*GetNameSafe(this),
				*GetNameSafe(PawnClass->GetClass()),
				Loc.X, Loc.Y, Loc.Z);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s: SpawnActorsFromSamples: finished. Spawned %d actors."), *GetNameSafe(this), SpawnedCount);
}

void AMobSpawnerRectangle::RemoveSpawnedActors()
{
	if (SpawnedActors.Num() == 0) return;

	for (AActor* A : SpawnedActors)
	{
		if (!IsValid(A)) continue;
		A->Destroy();
	}

	SpawnedActors.Empty();
}

bool AMobSpawnerRectangle::GetGroundZAtLocation(const FVector& Location, float& OutZ) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	FHitResult Hit;
	FVector Start = Location + FVector(0.f, 0.f, 1000.f);   // trace from above
	FVector End   = Location - FVector(0.f, 0.f, 1000.f);   // trace down

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GroundTrace), false);
	Params.AddIgnoredActor(this);

	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		OutZ = Hit.ImpactPoint.Z + SpawnHeightOffset;  // use editor-exposed offset
		return true;
	}

	return false;
}

void AMobSpawnerRectangle::UpdateSpawnedActorsRotation()
{
	if (!TargetSphere || SpawnedActors.Num() == 0) return;

	const FVector TargetLoc = TargetSphere->GetComponentLocation();

	for (AActor* Actor : SpawnedActors)
	{
		if (!IsValid(Actor)) continue;

		FVector Dir = TargetLoc - Actor->GetActorLocation();
		Dir.Z = 0; // keep actors upright
		if (!Dir.IsNearlyZero())
		{
			FRotator Rot = Dir.Rotation();
			Actor->SetActorRotation(Rot);
		}
	}
}

void AMobSpawnerRectangle::RandomizeSamplePoints()
{
#if WITH_EDITOR
	// Temporarily store debug flag
	bool bPrevDrawDebug = bDrawDebug;
	bDrawDebug = false;  // disable regular debug during generation

	// Remove any existing spawned actors (just in case)
	RemoveSpawnedActors();

	// Clear old persistent debug spheres
	FlushPersistentDebugLines(GetWorld());
	
	// Recalculate sample points
	SamplePoints = CalculateSamplePoints(bPerformCollisionCheck);

	UE_LOG(LogTemp, Log, TEXT("%s: RandomizeSamplePoints: %d sample points generated."), *GetNameSafe(this), SamplePoints.Num());

	// Draw persistent debug spheres at new sample points
	DrawPersistentDebugSpheres();

	// Restore debug flag
	bDrawDebug = bPrevDrawDebug;
#endif
}

void AMobSpawnerRectangle::RandomizeAndSpawnActors()
{
#if WITH_EDITOR
	bool bPrevDrawDebug = bDrawDebug;
	bDrawDebug = false; // disable debug during spawning

	RemoveSpawnedActors();

	
	SamplePoints = CalculateSamplePoints(bPerformCollisionCheck);
	SpawnActorsFromSamples();

	bDrawDebug = bPrevDrawDebug;
#endif
}