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


	UPROPERTY(BlueprintReadWrite, Category="Spawner")
	float SizeX_Default = 1000.f;

	UPROPERTY(BlueprintReadWrite, Category="Spawner")
	float SizeY_Default = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	FVector2D Size = FVector2D(SizeX_Default, SizeY_Default);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* Spline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner")
	bool ClosedLoopRequired = true;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
};
