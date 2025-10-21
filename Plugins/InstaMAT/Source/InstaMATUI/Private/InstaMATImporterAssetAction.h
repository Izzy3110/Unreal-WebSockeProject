/**
 * InstaMATImporterAssetAction.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterAssetAction.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMATImporterGraphInstanceAssetAction_h
#define InstaMAT_InstaMATImporterGraphInstanceAssetAction_h

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * The FInstaMATImporterGraphInstanceAssetAction applies asset actions for InstaMATGraphInstance objects.
 */
class FInstaMATImporterGraphInstanceAssetAction : public FAssetTypeActions_Base
{
	/**
	 * Creates a thumbnail overlay.
	 * 
	 * @param AssetData the asset data.
	 * @return the SWidget.
	 */
	virtual TSharedPtr<class SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;

	/**
	 * Retrieves the color for the highlights in the thumbnail.
	 *
	 * @return the color.
	 */
	virtual FColor GetTypeColor() const override;

	/**
	 * Gets the class this assets action applies to.
	 *
	 * @return the class.
	 */
	virtual UClass* GetSupportedClass() const override;

	/**
	 * Gets type name visible in the UI.
	 *
	 * @return the name.
	 */
	virtual FText GetName() const override;

	/**
	 * Gets the category for the object.
	 *
	 * @return the category.
	 */
	virtual uint32 GetCategories() override;

	/**
	 * Gets the actions for the specified \p InObjects.
	 *
	 * @param InObjects the selected objects in the Content Browser.
	 * @param Section the tool menu.
	 */
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;

	/**
	 * Determines whether actions apply.
	 *
	 * @param InObjects the selected objects in the Content Browser.
	 * @return true if actions apply.
	 */
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
};

/**
 * The FInstaMATImporterGraphAssetAction applies asset actions for InstaMATGraph objects.
 */
class FInstaMATImporterGraphAssetAction : public FAssetTypeActions_Base
{
	/**
	 * Creats a thumbnail overlay.
	 *
	 * @param AssetData the asset data.
	 * @return the SWidget.
	 */
	virtual TSharedPtr<class SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;

	/**
	 * Retrieves the color for the highlights in the thumbnail.
	 *
	 * @return the color.
	 */
	virtual FColor GetTypeColor() const override;

	/**
	 * Gets the class this assets action applies to.
	 *
	 * @return the class.
	 */
	virtual UClass* GetSupportedClass() const override;

	/**
	 * Gets type name visible in the UI.
	 *
	 * @return the name.
	 */
	virtual FText GetName() const override;

	/**
	 * Gets the category for the object.
	 *
	 * @return the category.
	 */
	virtual uint32 GetCategories() override;

	/**
	 * Gets the actions for the specified \p InObjects. 
	 * 
	 * @param InObjects the selected objects in the Content Browser.
	 * @param Section the tool menu.
	 */
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;

	/**
	 * Determines whether actions apply.
	 *
	 * @param InObjects the selected objects in the Content Browser.
	 * @return true if actions apply.
	 */
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
};

#endif