/**
 * InstaMATThumbnailRenderer.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATThumbnailRenderer.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "InstaMATThumbnailRenderer.generated.h"

class FCanvas;
class FRenderTarget;

/**
 * The UInstaMATImporterGraphAssetThumbnailRenderer provides functions to show 
 * the thumbnail for all objects implementing the InstaMATThumbnailInterface.
 */
UCLASS()
class INSTAMATUI_API UInstaMATThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_UCLASS_BODY()
public:

	/**
	 * Gets the thumbnail size.
	 *
	 * @param Object the object of the thumbnail.
	 * @param Zoom the zoom factor.
	 * @param [out] OutWidth the width.
	 * @param [out] OutHeight the height.
	 */
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;

	/**
	 * Draws the thumbnail.
	 *
	 * @param Object the object of the thumbnail.
	 * @param X the x coordinate.
	 * @param Y the y coordinate.
	 * @param Width the width.
	 * @param Height the height.
	 * @param Target the render target.
	 * @param Canvas the canvas.
	 * @param bAdditionalViewFamily.
	 */
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas, bool bAdditionalViewFamily) override;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
	/**
	 * Checks whether the object can be rendered with the thumbnail renderer.
	 * 
	 * @param Object The object.
	 * @return True if valid.
	 */
	virtual bool CanVisualizeAsset(UObject* Object) override;
#endif
};
