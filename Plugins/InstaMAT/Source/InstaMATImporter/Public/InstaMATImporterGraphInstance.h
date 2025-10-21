/**
 * InstaMATImporterGraphInstance.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraphInstance.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMAT/InstaMATEnum.h"
#include "InstaMATThumbnailInterface.h"
#include "Framework/Application/IInputProcessor.h"
#include "InstaMATImporterGraphInstance.generated.h"

class FInstaMATTextureDataSource;

/**
 * The UInstaMATImporterGraphInstance represents a InstaMAT Graph Asset.
 */
UCLASS(Blueprintable, hideCategories = Object)
class INSTAMATIMPORTER_API UInstaMATImporterGraphInstance : public UObject
{
	GENERATED_BODY()
public:

	UInstaMATImporterGraphInstance();
	~UInstaMATImporterGraphInstance();

	/**
	 * Updates this instance.
	 * 
	 * @param bOnlyUpdateGPU If enabled only updates the GPU textures.
	 */
	UFUNCTION(BlueprintCallable)
	void Update(const bool bOnlyUpdateGPU = false);

	/**
	 * Gets the sorted input categories, or recreates them
	 * if not available.
	 * 
	 * @return Array of sorted input categories.
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetSortedInputCategories();

	/**
	 * Saves the output images to disk for the graph instance.
	 *
	 * @param SaveOutputs whether the output should be exported.
	 * @param Directory the save directory.
	 * @param Format the execution format.
	 * @param Rotation the image rotation.
	 * @param FileType the image file type.
	 * @param Width the image width.
	 * @param Height the image height.
	 */
	void SaveOutputImagesToDisk(const TMap<const class UInstaMATOutput*, bool>& SaveOutputs, const FString& Directory, const EInstaMATExecutionFormat Format, const EInstaMATRotation Rotation, const EInstaMATTextureFileType FileType, const EInstaMATTextureSize Width, const EInstaMATTextureSize Height) const;

	/**
	 * Saves the output images to disk for the graph instance.
	 *
	 * @param SaveOutputs whether the output should be exported.
	 * @param Directory the save directory.
	 * @param ExportSettings the output settings.
	 */
	void SaveOutputImagesToDisk(const TMap<const class UInstaMATOutput*, bool>& SaveOutputs, const FString& Directory, const struct FInstaMATExportTextureSettings& ExportSettings) const;

	/**
	 * Function to set the instance dirty.
	 * 
	 * @param bIsDirty whether to set this instance dirty.
	 * @param bIgnoreNextPropertyEvent whether the next property event shall be ignored.
	 */
	void SetDirty(bool bIsDirty, bool bIgnoreNextPropertyEvent = false);

	/**
	 * Function is called when the object is destroyed.
	 */
	virtual void BeginDestroy() override;

	/**
	 * Function is called if the property system determines a value changed.
	 *
	 * @param PropertyChangedEvent information object.
	 */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (PropertyChangedEvent.Property == nullptr)
			return;

		if (EnsurePropertyChangeRequirementsAreApplied(PropertyChangedEvent))
			return;

		UObject::PostEditChangeProperty(PropertyChangedEvent);

		if (DefaultSeed == -1)
		{
			Seed = FMath::Clamp(Seed, 0, 100000);
		}

		if (!bIgnoreNextPropertyEvent)
		{
			SetDirty(true);
		}
		bIgnoreNextPropertyEvent = false;
	}

	/**
	 * Handles property change validation.
	 * 
	 * @param PropertyChangedEvent information object.
	 * @return If the return value is true the update event must be suppressed.
	 */
	bool EnsurePropertyChangeRequirementsAreApplied(struct FPropertyChangedEvent& PropertyChangedEvent);

	/**
	 * Retrieves the inputs of this instance that have the specified \p Category.
	 *
	 * @param Category the category.
	 * @return The input objects.
	 */
	TArray<UInstaMATInputBase*> GetInputsByCategory(const FString& Category);

	/**
	 * Checks if this instance requires to reallocate the element execution.
	 * 
	 * @param True if element execution is required to be reallocated.
	 */
	bool IsElementExecutionReallocationRequired() const;

	/**
	 * Sets whether the instance needs to reallocate the element execution.
	 * 
	 * @param bIsRequired Whether it is required.
	 */
	void SetElementExecutionReallocationRequired(bool bIsRequired);

	/**
	 * Determines whether the format settings are dirty.
	 *
	 * @return True if format settings are dirty.
	 */
	bool IsFormatSettingsDirty() const;

	/**
	 * Sets whether the format settings are dirty.
	 *
	 * @param bIsDirty Whether the format settings are dirty.
	 */
	void SetFormatSettingsDirty(bool bIsDirty);

	/**
	 * Updates the material instance.
	 * 
	 * @param PropertyChangedEvent information about which properties changed.
	 */
	void UpdateMaterialInstanceSettings(struct FPropertyChangedEvent& PropertyChangedEvent);

	/**
	 * Sets the texture source data for an output parameter.
	 * 
	 * @param OutputName the name of the output parameter.
	 * @param TextureDataSource the texture source data used to update the UE resources.
	 */
	void SetOutputTextureSourceData(const FString& OutputName, const TSharedPtr<FInstaMATTextureDataSource>& TextureDataSource);

	/**
	 * Gets the input with the specified \p Name.
	 * @note will return nullptr if not found.
	 * 
	 * @param Name the input name.
	 * @return the Input object.
	 */
	UFUNCTION(BlueprintCallable)
	UInstaMATInputBase* GetInputParameterByName(const FString& Name);

	/** The graph id for this instance. */
	UPROPERTY(BlueprintReadOnly)
	FString GraphID;

	/** The graph friendly name of this instance. */
	UPROPERTY(BlueprintReadOnly, Category = "Information")
	FString GraphFriendlyName;

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

	/** The tags of this graph. */
	UPROPERTY(VisibleAnywhere, Category = "Information")
	FString Tags;

	/** The preview image rendered by InstaMAT.*/
	UPROPERTY()
	class UTexture2D* PreviewImage;

	/** The preview image brush. */
	FSlateBrush* PreviewBrush;

	/** This is the custom name given by the user when instantiating. */
	UPROPERTY(BlueprintReadOnly)
	FString CustomName;

	/** The source import file path. */
	UPROPERTY(BlueprintReadOnly)
	FString ImportFilePath;

	/** The random seed of this instance. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InstanceProperties", Meta = ( ClampMin = "0", ClampMax = "100000", UIMin = "0", UIMax = "100000"))
	int32 Seed;

	/** The rotation of this instance. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InstanceProperties")
	EInstaMATRotation Rotation;

	/** The displacement height of this instance. */
	UPROPERTY(BlueprintReadOnly)
	float DisplacementHeight;

	/** Whether this instance is a grayscale permutation. */
	UPROPERTY(EditAnywhere, Category = "InstanceProperties")
	bool bIsGrayScalePermutation = false;

	/** Determines whether this instance supports grayscale permutation. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsGrayScalePermutable = false;

	/** Whether this instance is currently rendering in preview mode. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InstanceMode", DisplayName="Enable Preview Mode")
	bool bIsPreviewMode = false;

	/** Specifies if the instance should update manually or automatically. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InstanceMode", Meta = (DisplayName = "Update Type", NoResetToDefault))
	EInstaMATUpdateType UpdateType = EInstaMATUpdateType::InstaMAT_Automatic;

	/** The default seed of this instance. */
	UPROPERTY(BlueprintReadOnly)
	int32 DefaultSeed;

	/** The scale on the U axis in UV space. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MaterialSettings")
	float ScaleU = 1.0f;

	/** The scale on the V axis in UV space. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MaterialSettings")
	float ScaleV = 1.0f;

	/** Enables the physical size on the material. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MaterialSettings")
	bool bEnablePhysicalSize = false;

	/** Enables triplanar texture mapping on the materials. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MaterialSettings")
	bool bEnableWorldAlignedTextures = false;

	/** The instance texture resolution. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ElementFormat", Meta = (EditCondition="!bIsPreviewMode"))
	EInstaMATTextureSize ResolutionWidth;

	/** The instance texture resolution. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ElementFormat", Meta = (EditCondition = "!bIsPreviewMode"))
	EInstaMATTextureSize ResolutionHeight;

	/** The execution format. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ElementFormat", Meta = (DisplayName="Texture Format", EditCondition = "!bIsPreviewMode"))
	EInstaMATExecutionFormat ExecutionFormat = EInstaMATExecutionFormat::InstaMAT_Normalized8;

	/** The input parameters of this instance. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "InputParameters")
	TArray<class UInstaMATInputBase*> InputParameters;

	/** The output texture parameters of the InstaMAT object. */
	UPROPERTY(BlueprintReadOnly)
	TArray<class UInstaMATOutput*> OutputParameters;

	/** The output mesh parameters of the InstaMAT object. */
	UPROPERTY(BlueprintReadOnly)
	TArray<class UInstaMATMeshOutput*> OutputMeshParameters;

	/** The output scene parameters of the InstaMAT object. */
	UPROPERTY(BlueprintReadOnly)
	TArray<class UInstaMATGraphSceneOutput*> OutputSceneParameters;

	/** The material instance bound to this object. */
	UPROPERTY()
	class UMaterialInstanceConstant* MaterialInstance;

	/** Sorted array of categories for inputs. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> InputCategoriesSorted;

	/** The physical size width. */
	UPROPERTY(BlueprintReadOnly, Meta = (EditCondition = "bHasPhysicalSize", EditConditionHides))
	float PhysicalWidth = 0;

	/** The physical size height. */
	UPROPERTY(BlueprintReadOnly, Meta = (EditCondition = "bHasPhysicalSize", EditConditionHides))
	float PhysicalHeight = 0;

	/** Whether this instance has physical size data. */
	UPROPERTY(BlueprintReadOnly)
	bool bHasPhysicalSize;

	/** Whether the graph is a material graph. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsMaterialGraph;

	/** Whether the texture is gray scale. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsBaseColorGrayscale = false;

	/** Fast Timer Handle for updating. */
	FTimerHandle FastUpdateTimerHandle;

	/** Timer Handle for updating. */
	FTimerHandle UpdateTimerHandle;

	/** The last time the dirty state changed. */
	float LastDirtyStateChanged;

	/** Determines whether an input had any changes to it's values. */
	bool bIsDirty = false;

	/** Determines whether the next property change will result in calling the update function. */
	bool bIgnoreNextPropertyEvent = false;

	/** Dirty flag for format settings change. */
	bool bIsFormatSettingsDirty = false;

	/** Dirty flag if the execution needs to be reallocated. */
	bool bIsElementExecutionRequiredReallocation = false;

	/** Unique identifier for this instance. */
	UPROPERTY()
	FGuid UUID;

private:

	TMap<FString, TSharedPtr<FInstaMATTextureDataSource>> OutputParametersTextureDataSource;	/**< The output texture data sources. */
};
