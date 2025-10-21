/**
 * InstaMATGraphSceneOutput.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATGraphSceneOutput.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMATGraphSceneOutput.generated.h"

/**
 * The UInstaMATGraphSceneOutput represents a InstaMAT Graph scene asset output.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATGraphSceneOutput : public UObject
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

	/** Scene materials. */
	UPROPERTY(BlueprintReadOnly)
	TArray<UMaterialInstanceConstant*> Materials;

	/** Scene textures. */
	UPROPERTY(BlueprintReadOnly)
	TArray<UTexture2D*> Textures;

	/** Scene meshes. */
	UPROPERTY(BlueprintReadOnly)
	TArray<UStaticMesh*> Meshes;

	/** The root node of this instance. */
	UPROPERTY(BlueprintReadOnly)
	class UInstaMATGraphSceneOutputNode* Root;

	/**
	 * Spawns the scene actors based on the scene hierarchy.
	 */
	void SpawnActorsInScene();
};