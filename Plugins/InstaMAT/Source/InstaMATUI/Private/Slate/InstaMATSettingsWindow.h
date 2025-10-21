/**
 * InstaMATWindow.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATWindow.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMATSettingsWindow_h
#define InstaMAT_InstaMATSettingsWindow_h

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

DECLARE_MULTICAST_DELEGATE(FOnNewSelection)

/**
 * The SInstaMATSettingsWindow is placed inside of a dockable tab.
 * All information and tools will be displayed inside of this window 
 * using a DetailView and Customizations.
 */
class SInstaMATSettingsWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInstaMATSettingsWindow) {}
	SLATE_END_ARGS()
 
	SInstaMATSettingsWindow();
	~SInstaMATSettingsWindow();

	/**
	 * Constructs the view of this instance.
	 *
	 * @param InArgs the construction arguments.
	 */
	void Construct(const FArguments& InArgs);

	/** Updates the contents of the UI. */
	void UpdateUIContent();

	/**
	 * Forces a redraw of this instance.
	 */
	void ForceRefreshDetailsView();
	
	/**
	 * Creates a footer textbox.
	 * 
	 * @param CustomText custom text for the footer. If empty, the default value will be shown.
	 * @return Footer textbox.
	 */
	static TSharedRef<STextBlock> CreateFooterTextBlock(const FString& CustomText = FString())
	{
		if (CustomText.IsEmpty())
		{
			return SNew(STextBlock)
				.Text(NSLOCTEXT("InstaMATUI", "WindowFooter",
					"InstaMaterial GmbH 2018 - 2025\n"
					"http://www.InstaMaterial.com\n"))
				.Justification(ETextJustify::Center);
		}

		return SNew(STextBlock)
			.Text(FText::FromString(CustomText))
			.Justification(ETextJustify::Center); 
	}

private:
	TSharedPtr<class IDetailsView> DetailView;	/**< Detail Viewer that shows the details of the used tools (UObject). */
};

#endif
