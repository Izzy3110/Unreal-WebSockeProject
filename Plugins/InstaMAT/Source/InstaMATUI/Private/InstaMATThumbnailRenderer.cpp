/**
 * InstaMATThumbnailRenderer.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATThumbnailRenderer.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATThumbnailRenderer.h"
#include "InstaMATImporter/Public/InstaMATThumbnailInterface.h" 

UInstaMATThumbnailRenderer::UInstaMATThumbnailRenderer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UInstaMATThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	check(Object != nullptr);
	if (IInstaMATThumbnailInterface* const Asset = Cast<IInstaMATThumbnailInterface>(Object))
	{
		Super::GetThumbnailSize(Asset->GetThumbnailImage(), Zoom, OutWidth, OutHeight);
	}
}

void UInstaMATThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	check(Object != nullptr);
	if (IInstaMATThumbnailInterface* const Asset = Cast<IInstaMATThumbnailInterface>(Object))
	{
		Super::Draw(Asset->GetThumbnailImage(), X, Y, Width, Height, Target, Canvas, bAdditionalViewFamily);
	}
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
bool UInstaMATThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	if (Object == nullptr || !Object->GetClass()->ImplementsInterface(UInstaMATThumbnailInterface::StaticClass()))
		return false;

	return true;
}
#endif
