/**
 * InstaMATImporterGraphCustomization.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraphCustomization.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMATImporterGraphCustomization_h
#define InstaMAT_InstaMATImporterGraphCustomization_h

#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

/**
 * The FInstaMATImporterGraphCustomization creates the layout in the
 * Detail Panel for the InstaMATimporterGraph object.
 */
class FInstaMATImporterGraphCustomization : public IDetailCustomization
{
public:

	FInstaMATImporterGraphCustomization();
	virtual ~FInstaMATImporterGraphCustomization();

	/**
	 * This function is called if the InstaMATImporterGraphAsset object is selected.
	 *
	 * @param DetailBuilder the detailbuilder.
	 */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	/**
	 * This function is called when the customization will be deleted.
	 */
	virtual void PendingDelete() override;

protected:
	
	IDetailLayoutBuilder* DetailLayoutBuilder; /**< Detail Layout builder instance. */
};

#endif