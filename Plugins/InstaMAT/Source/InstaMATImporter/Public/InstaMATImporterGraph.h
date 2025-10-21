/**
 * InstaMATImporterGraph.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraph.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMATThumbnailInterface.h"
#include "InstaMATModule.h"
#include "InstaMATImporterFactory.h"
#include "InstaMATImporterGraph.generated.h"

// Forward declaration.
class UInstaMATImporterGraphInstance;

 /**
  * The UInstaMATImporterGraph can create InstaMATImporterGraphInstances
  */
UCLASS(hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATImporterGraph : public UObject, public IInstaMATThumbnailInterface
{
	GENERATED_BODY()
public:
	UInstaMATImporterGraph();

	virtual TObjectPtr<UTexture2D> GetThumbnailImage(bool bIsForcingGeneration = false) override
	{
		if (ThumbnailImageCache == nullptr || bIsForcingGeneration)
		{
			RegenerateThumbnailImage();
		}

		return ThumbnailImageCache;
	};

	virtual void RegenerateThumbnailImage() override
	{
		FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));

		const FString PreviewImageName = FString::Format(TEXT("{0}_preview"), { GetName() });
		ThumbnailImageCache = FInstaMATImporterUtility::CreatePreviewImage(InstaMATModule, nullptr, TEXT(""), PreviewImageName, GraphID, /*bUseAlpha:*/ false);
	}

	/** The graph id for this instance. */
	UPROPERTY()
	FString GraphID;

	/** The graph friendly name of this instance. */
	UPROPERTY()
	FString GraphFriendlyName; 

	/** The source import file path. */
	UPROPERTY()
	FString ImportFilePath;

	/** The new instance name. */
	UPROPERTY()
	FString NewInstanceName;

	/** MetaData information describing the category. */
	UPROPERTY(VisibleAnywhere, Category = "Information")
	FString Category;

	/** MetaData information describing the underlying GraphObject. */
	UPROPERTY(VisibleAnywhere, Category = "Information")
	FString Documentation;

	/** MetaData information describing the author. */
	UPROPERTY(VisibleAnywhere, Category = "Information")
	FString Author;

	/** MetaData information describing the URL. */
	UPROPERTY(VisibleAnywhere, Category = "Information")
	FString URL;

	/** Information describing the Version. */
	UPROPERTY(VisibleAnywhere, Category = "Information")
	FString Version;

	/** The tags of the graph. */
	UPROPERTY(VisibleAnywhere, Category = "Information")
	FString Tags;
	/** Whether the graph is a material graph. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsMaterialGraph;

	/** The preview image rendered by InstaMAT.*/
	UPROPERTY(SkipSerialization)
	class UTexture2D* PreviewImage;

	/** The last generated Thumbnail image, cached for performance improvements. */
	TObjectPtr<UTexture2D> ThumbnailImageCache;

	/** The preview image brush. */
	FSlateBrush* PreviewBrush;

	/**
	 * Create a new instance of the graph with the set custom name. 
	 * 
	 * @param TargetDirectoryPath	Optional. The target folder path to of the new Instance. If empty, Instance will be created in the same folder as the Graph.
	 * @return The created object.
	 */
	UInstaMATImporterGraphInstance* CreateNewInstance(const FString& TargetDirectoryPath = FString());

	/**
	 * Determines whether the specified \p CustomName is valid.
	 *
	 * @param Path The object path.
	 * @param CustomName The provided custom name.
	 * @return True if valid.
	 */
	static bool IsCustomNameValid(const FString& Path, const FString& CustomName);
};
