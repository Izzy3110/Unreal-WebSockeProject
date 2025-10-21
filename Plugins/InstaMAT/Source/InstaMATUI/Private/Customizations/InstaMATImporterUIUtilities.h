/**
 * InstaMATImporterUIUtilities.h (InstaMAT)
 *
 * Copyright 2019-2021 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterUIUtilities.h
 * @copyright 2019-2021 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMATImporterUIUtilities_h
#define InstaMAT_InstaMATImporterUIUtilities_h

#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"

/**
 * The FInstaMATImporterUIUtilities contains utility UI functions.
 */
namespace FInstaMATImporterUIUtilities
{
	const FText gInputNotSupportedText = NSLOCTEXT(LOCTEXT_NAMESPACE,
		"InstaMAT_InputNotSupported",
		"Input type not supported on Unreal Engine.");

	const FText gInputNotSupportedTextTooltip = NSLOCTEXT(LOCTEXT_NAMESPACE,
		"InstaMAT_InputNotSupportedTooltip",
		"Some input types of Graphs are not supported on Unreal Engine.\n"
		"You can change the visibility of this Input parameter in InstaMAT Settings page under 'UI Settings' category.");

	/**
	 * Renders the meta data panel with the specified strings.
	 *
	 * @param Name the graph name.
	 * @param Category the graph category.
	 * @param Documentation the graph documentation.
	 * @param Author the graph author.
	 * @param URL the graph URL.
	 * @param Version the graph version.
	 * @param Tags the graph tags.
	 * @return The panel.
	 */
	TSharedRef<SVerticalBox> CreateMetaDataPanel(const FString& Name, const FString& Category, const FString& Documentation, const FString& Author, const FString& URL, const FString& Version, const FString& Tags);

	/**
	 * Gets the matching text color for the specified \p BackgroundColor.
	 *
	 * @param BackgroundColor The background color.
	 * @return The linear color.
	 */
	static FLinearColor GetTextColorForBackgroundColor(const FSlateRoundedBoxBrush* const BackgroundColor);

	/**
	 * Creates tags for the specified tags string.
	 *
	 * @param The tags string.
	 */
	SVerticalBox::FSlot::FSlotArguments CreateTags(const FString& Tags);
};

#endif /*InstaMAT_InstaMATImporterUIUtilities_h*/