/**
 * InstaMATImporterGraphInstanceCustomization.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraphInstanceCustomization.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMATImporterGraphInstanceCustomization_h
#define InstaMAT_InstaMATImporterGraphInstanceCustomization_h

#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

/**
 * The FInstaMATImporterGraphInstanceCustomization creates the layout in the
 * Detail Panel for the InstaMATimporterGraphInstance object.
 */
class FInstaMATImporterGraphInstanceCustomization : public IDetailCustomization
{
public:

	FInstaMATImporterGraphInstanceCustomization();
	virtual ~FInstaMATImporterGraphInstanceCustomization();

	/**
	 * This function is called if the InstaMATImporterGraphInstance object is selected.
	 *
	 * @param DetailBuilder the detailbuilder.
	 */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	/**
	 * This function is called when the customization will be deleted.
	 */
	virtual void PendingDelete() override;

	/**
	 * Opens a save textures dialog for the specified \p Instance.
	 * 
	 * @param Instance the instance.
	 */
	static void ShowSaveTexturesDialog(const class UInstaMATImporterGraphInstance* const Instance);

	/**
	 * Calls the refresh update method of the detail layout builder.
	 */
	void ForceRefreshUpdate();

protected:
	
	IDetailLayoutBuilder* DetailLayoutBuilder; /**< Detail Layout builder instance. */
};

#endif