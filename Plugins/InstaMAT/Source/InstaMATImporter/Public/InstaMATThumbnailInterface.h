/**
 * InstaMATThumbnailInterface.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATThumbnailInterface.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMATThumbnailInterface.generated.h"

/**
 * The UInstaMATThumbnailInterface provides provides the UObject base for the thumbnail interface.
 */
UINTERFACE()
class INSTAMATIMPORTER_API UInstaMATThumbnailInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * The IInstaMATThumbnailInterface provides functions to retrieve the thumbnail image.
 */
class INSTAMATIMPORTER_API IInstaMATThumbnailInterface
{
	GENERATED_BODY()
public:

	/**
	 * Retrieves the thumbnail image from the cache or generates it when it hasn't been cached yet.
	 * 
	 * @param bIsForcingNew	(optional) Set to true to regenerate the thumbnail and update the cache.
	 * @return The thumbnail image.
	 */
	virtual TObjectPtr<UTexture2D> GetThumbnailImage(bool bIsForcingNew = false) = 0;
	
	/** Function to generate a new thumbnail image. */
	virtual void RegenerateThumbnailImage() = 0;
};