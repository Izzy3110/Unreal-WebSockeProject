/**
 * InstaMATOutput.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATOutput.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMATOutput.generated.h"

/**
 * The UInstaMATOutput defines a texture output.
 */
UCLASS(hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATOutput : public UObject
{
	GENERATED_BODY()
public:
	/** The output name. */
	UPROPERTY()
	FString OutputName;
	
	/** The graph id of the parent InstaMAT GraphObject. */
	UPROPERTY()
	FString ParentGraphID;
	
	/** The parameter index in the InstaMAT GraphObject. */
	UPROPERTY()
	uint32 Index;
	
	/** The color channel count for the output. */
	UPROPERTY()
	uint32 ColorChannelCount;
	
	/** The result texture output. */
	UPROPERTY()
	class UTexture2D* Output;
	
	/** For painting in UI. */
	TSharedPtr<FSlateDynamicImageBrush> Brush;
};