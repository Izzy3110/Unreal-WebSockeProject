/**
 * InstaMATImporterGraphInstanceThumbnailRenderer.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraphInstanceThumbnailRenderer.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/ThumbnailRenderer.h"
#include "InstaMATImporterGraphInstanceThumbnailRenderer.generated.h"

class FCanvas;
class FRenderTarget;

/**
 * The UInstaMATImporterGraphInstanceThumbnailRenderer provides functions  
 * to render a thumbnail for the InstaMATImporterGraphInstance objects.
 */
UCLASS()
class INSTAMATUI_API UInstaMATImporterGraphInstanceThumbnailRenderer : public UThumbnailRenderer
{
	GENERATED_UCLASS_BODY()
public:

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

	/**
	 * Function invoked before deallocating this instance. 
	 */
	virtual void BeginDestroy() override;

private:
	
	class FMaterialThumbnailScene* ThumbnailScene;	/**< The thumbnail scene. */
};
