/**
 * InstaMATSettingsCustomization.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSettingsCustomization.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMATSettingsCustomization_h
#define InstaMAT_InstaMATSettingsCustomization_h

#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

/**
 *	The FInstaMATSettingsCustomization shows a custom UI for The UInstaMATSettings class.
 */
class FInstaMATSettingsCustomization : public IDetailCustomization
{
public:
	FInstaMATSettingsCustomization();
	~FInstaMATSettingsCustomization();
	
	/**
	 * Called when something wants to refresh the DetailsView. 
	 */
	void OnForceRefreshDetails();

	/**
	 * Called when the Matching Button is pressed.
	 * 
	 * @param DetailBuilder the detail layout builder.
	 * @param MethodToExecute the method to execute.
	 * @return The Reply object.
	 */
	static FReply ExecuteTool(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute);
	
	/**
	 * Called when the AccountName box is changed.
	 * 
	 * @param NewText the new text.
	 */
	void OnAccountNameChanged(const FText& NewText);

	/**
	 * Called when the Password box is changed.
	 *
	 * @param NewText the new text.
	 */
	void OnPasswordChanged(const FText& NewText);
	
	/**
	 * Customizes the view.
	 *
	 * @param DetailBuilder the detail layout builder.
	 */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	/**
	 * Called when the environment path is selection button is clicked.
	 *
	 * @return The Reply object.
	 */
	FReply OnEnvironmentPathSelectionButtonClicked();

	/**
	 * Handles the ingest license button click.
	 * 
	 * @return The Reply object.
	 */
	FReply OnIngestLicenseButtonClicked();

private:
	class UInstaMATSettings* Settings;					/**< The settings object. */
	IDetailLayoutBuilder* DetailLayoutBuilder;			/**< The detail layout builder. */
	FSimpleDelegate RefreshDetailsDelegate;				/**< The delegate to invoke refreshing from external code. */
};

#endif
