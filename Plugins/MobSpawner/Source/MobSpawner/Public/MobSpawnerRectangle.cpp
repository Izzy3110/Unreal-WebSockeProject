// MobSpawnerRectangle.cpp
#include "MobSpawnerRectangle.h"
#include "Components/SplineComponent.h"

AMobSpawnerRectangle::AMobSpawnerRectangle()
{
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;
}

void AMobSpawnerRectangle::OnConstruction(const FTransform& Transform)
{
	if (!Spline) return;

	Spline->ClearSplinePoints(false);

	const FVector Half = FVector(Size.X / 2, Size.Y / 2, 0.f);

	Spline->AddSplinePoint(FVector(-Half.X, -Half.Y, 0), ESplineCoordinateSpace::Local);
	Spline->AddSplinePoint(FVector(Half.X, -Half.Y, 0), ESplineCoordinateSpace::Local);
	Spline->AddSplinePoint(FVector(Half.X, Half.Y, 0), ESplineCoordinateSpace::Local);
	Spline->AddSplinePoint(FVector(-Half.X, Half.Y, 0), ESplineCoordinateSpace::Local);
	
	if (ClosedLoopRequired)
	{
		if (Spline->IsClosedLoop() == false)
			Spline->SetClosedLoop(true);
	}
	
	Spline->UpdateSpline();
}
