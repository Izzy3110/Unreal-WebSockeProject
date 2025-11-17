#include "MobSpawnerRectangle.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

AMobSpawnerRectangle::AMobSpawnerRectangle()
{
	PrimaryActorTick.bCanEverTick = true; // needed for runtime debug updates

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;
}

void AMobSpawnerRectangle::OnConstruction(const FTransform& Transform)
{
	if (!Spline) return;

	Spline->ClearSplinePoints(false);

	const FVector Half = FVector(Size.X / 2.f, Size.Y / 2.f, 0.f);

	Spline->AddSplinePoint(FVector(-Half.X, -Half.Y, 0.f), ESplineCoordinateSpace::Local);
	Spline->AddSplinePoint(FVector(Half.X, -Half.Y, 0.f), ESplineCoordinateSpace::Local);
	Spline->AddSplinePoint(FVector(Half.X, Half.Y, 0.f), ESplineCoordinateSpace::Local);
	Spline->AddSplinePoint(FVector(-Half.X, Half.Y, 0.f), ESplineCoordinateSpace::Local);

	if (ClosedLoopRequired && !Spline->IsClosedLoop())
	{
		Spline->SetClosedLoop(true);
	}

	Spline->UpdateSpline();

	// Draw debug in the editor or runtime if enabled
	if (bDrawDebug)
	{
		DrawDebugRectangle();
	}
}

void AMobSpawnerRectangle::BeginPlay()
{
	Super::BeginPlay();

	// Draw debug immediately at "BeginPlay" if enabled
	if (bDrawDebug)
	{
		DrawDebugRectangle();
	}
}

void AMobSpawnerRectangle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Continuous runtime debug
	if (bDrawDebug && bDrawDebugEveryTick)
	{
		DrawDebugRectangle();
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
		DrawDebugSphere(GetWorld(), Point, DebugPointSize, 12, bInside ? FColor::Green : FColor::Red, false, 2.f);
	}

	return bInside;
}

FVector AMobSpawnerRectangle::GetRandomPointInsideSpline() const
{
	const float HalfX = Size.X / 2.f;
	const float HalfY = Size.Y / 2.f;

	const float RandomX = FMath::FRandRange(-HalfX, HalfX);
	const float RandomY = FMath::FRandRange(-HalfY, HalfY);

	const FVector RandomPoint = GetActorTransform().TransformPosition(FVector(RandomX, RandomY, 0.f));

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), RandomPoint, DebugPointSize, 12, FColor::Red, false, 2.f);
	}

	return RandomPoint;
}

void AMobSpawnerRectangle::DrawDebugRectangle() const
{
	if (!bDrawDebug) return;

	const FVector Half = FVector(Size.X / 2.f, Size.Y / 2.f, 0.f);
	const FVector WorldPoints[4] = {
		GetActorTransform().TransformPosition(FVector(-Half.X, -Half.Y, 0.f)),
		GetActorTransform().TransformPosition(FVector(Half.X, -Half.Y, 0.f)),
		GetActorTransform().TransformPosition(FVector(Half.X, Half.Y, 0.f)),
		GetActorTransform().TransformPosition(FVector(-Half.X, Half.Y, 0.f))
	};

	// Draw rectangle edges
	for (int32 i = 0; i < 4; i++)
	{
		DrawDebugLine(GetWorld(), WorldPoints[i], WorldPoints[(i + 1) % 4], DebugColor, false, 0.f, 0, 2.f);
	}
}
