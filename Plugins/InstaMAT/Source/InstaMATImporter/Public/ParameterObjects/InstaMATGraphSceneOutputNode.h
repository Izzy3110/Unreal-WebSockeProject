/**
 * InstaMATGraphSceneOutputNode.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATGraphSceneOutputNode.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMATGraphSceneOutputNode.generated.h"

/**
 * The UInstaMATGraphSceneOutputNode represents a InstaMAT Graph scene node.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATGraphSceneOutputNode : public UObject
{
	GENERATED_BODY()
public:

	/** The transformation of this instance. */
	UPROPERTY(BlueprintReadOnly)
	FTransform Transform;

	/** The static mesh of this instance. */
	UPROPERTY(BlueprintReadOnly)
	UStaticMesh* Mesh;

	/** The children of this instance. */
	UPROPERTY(BlueprintReadOnly)
	TArray<UInstaMATGraphSceneOutputNode*> Children;

	/** The node name. */
	UPROPERTY(BlueprintReadOnly)
	FString NodeName;

	/** The full scene path. */
	UPROPERTY(BlueprintReadOnly)
	FString ScenePath;

	/**
	 * Clears the hierarchy. 
	 */
	void Clear()
	{
		Mesh = nullptr;

		// Apply clear on children.
		for (UInstaMATGraphSceneOutputNode* const Child : Children)
		{
			Child->Clear();
		}

		Children.Empty();

		// Begin delete
		RemoveFromRoot();
		ConditionalBeginDestroy();
	}
};