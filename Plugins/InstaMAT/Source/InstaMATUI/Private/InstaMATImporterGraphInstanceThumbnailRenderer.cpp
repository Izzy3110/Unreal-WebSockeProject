/**
 * InstaMATImporterGraphInstanceThumbnailRenderer.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraphInstanceThumbnailRenderer.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATImporterGraphInstanceThumbnailRenderer.h"
#include "ThumbnailHelpers.h"
#include "ThumbnailRendering/MaterialInstanceThumbnailRenderer.h"
#include "InstaMATImporter/Public/InstaMATImporterGraphInstance.h" 
 
UInstaMATImporterGraphInstanceThumbnailRenderer::UInstaMATImporterGraphInstanceThumbnailRenderer(const FObjectInitializer& ObjectInitializer) : 
Super(ObjectInitializer),
ThumbnailScene(nullptr)
{
}

void UInstaMATImporterGraphInstanceThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	check(Object != nullptr);
	UInstaMATImporterGraphInstance* const Asset = Cast<UInstaMATImporterGraphInstance>(Object);
	if (Asset == nullptr)
		return;

	UMaterialInstance *const MaterialInstance = Asset->MaterialInstance;
	if (MaterialInstance == nullptr)
		return;

	if (ThumbnailScene == nullptr || ensure(ThumbnailScene->GetWorld() != nullptr) == false)
	{
		if (ThumbnailScene)
		{
			FlushRenderingCommands();
			delete ThumbnailScene;
			ThumbnailScene = nullptr;
		}

		ThumbnailScene = new FMaterialThumbnailScene();
	}

	check(ThumbnailScene != nullptr);

	ThumbnailScene->SetMaterialInterface(MaterialInstance);
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
		.SetTime(FGameTime::GetTimeSinceAppStart())
		.SetAdditionalViewFamily(bAdditionalViewFamily));

	ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
	ViewFamily.EngineShowFlags.SetSeparateTranslucency(ThumbnailScene->ShouldSetSeparateTranslucency(MaterialInstance));
	ViewFamily.EngineShowFlags.MotionBlur = false;
	ViewFamily.EngineShowFlags.AntiAliasing = false;
	RenderViewFamily(Canvas, &ViewFamily, ThumbnailScene->CreateView(&ViewFamily, X, Y, Width, Height));
	ThumbnailScene->SetMaterialInterface(nullptr);
}

void UInstaMATImporterGraphInstanceThumbnailRenderer::BeginDestroy()
{
	if (ThumbnailScene != nullptr)
	{
		delete ThumbnailScene;
		ThumbnailScene = nullptr;
	}

	Super::BeginDestroy();
}
