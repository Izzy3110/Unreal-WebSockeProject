/**
 * InstaMATMeshOutput.h (InstaMAT)
 *
 * Copyright 2019-2021 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATMeshOutput.h
 * @copyright 2019-2021 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"

#include "InstaMATMeshOutput.generated.h"

/**
 * The UInstaMATMeshOutput defines a Mesh Output.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATMeshOutput : public UObject
{
	GENERATED_BODY()
public:
	/** The output name. */
	UPROPERTY(BlueprintReadOnly)
	FString OutputName;
	
	/** The graph id of the parent InstaMAT GraphObject. */
	UPROPERTY(BlueprintReadOnly)
	FString ParentGraphID;
	
	/** The parameter index in the InstaMAT GraphObject. */
	UPROPERTY(BlueprintReadOnly)
	int32 Index;
	
	/** The result mesh output. */
	UPROPERTY(BlueprintReadOnly)
	class UStaticMesh* Output;
	
	/** For painting in UI. */
	TSharedPtr<FSlateBrush> Brush;
};