/**
 * SInstaMATGraphLibraryPreviewWidget.h (InstaMAT)
 *
 * Copyright 2019-2022 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file SInstaMATGraphLibraryPreviewWidget.h
 * @copyright 2019-2022 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMATGraphLibraryPreviewWidget_h
#define InstaMAT_InstaMATGraphLibraryPreviewWidget_h

#include "Widgets/SCompoundWidget.h"
#include "InstaMATModule.h"

/**
 * The SInstaMATGraphLibraryPreviewWidget shows a preview item in the graph library browser.
 */
class SInstaMATGraphLibraryPreviewWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SInstaMATGraphLibraryPreviewWidget)
	{}
	SLATE_END_ARGS()

public:
	SInstaMATGraphLibraryPreviewWidget();
	~SInstaMATGraphLibraryPreviewWidget();

	/**
	 * Constructs the view of this instance.
	 *
	 * @param InArgs the construction arguments.
	 */
	void Construct(const FArguments& InArgs);

	/**
	 * Sets the specified \p PreviewItem to this instance
	 * and updates the view.
	 *
	 * @param PreviewItem the preview item.
	 */
	void SetPreviewItem(TSharedPtr<FInstaMATGraphObjectViewItem>& PreviewItem);

	/**
	 * Gets the preview item of this instance.
	 * 
	 * @return The preview item.
	 */
	TSharedPtr<FInstaMATGraphObjectViewItem> GetPreviewItem();

private:

	/**
	 * Creates a graph input information row.
	 *
	 * @param Value The graph input.
	 * @param NameLabelWidth The width of the input name label.
	 * @return The information row layout.
	 */
	static TSharedRef<SHorizontalBox> CreateGraphInputInformationRow(const FInstaMATGraphObjectInputData& Value, const float NameLabelWidth);

	/**
	 * Creates a graph output information row.
	 * 
	 * @param Value The graph output.
	 * @param NameLabelWidth The width of the output name label.
	 * @return The information row layout.
	 */
	static TSharedRef<SHorizontalBox> CreateGraphOutputInformationRow(const FInstaMATGraphObjectOutputData& Value, const float NameLabelWidth);

	TSharedPtr<FInstaMATGraphObjectViewItem> PreviewItem;	/**< The Preview Item. */
	TSharedPtr<FSlateColorBrush> BorderBrush;				/**< Default brush for input category header. */
};

#endif /*InstaMAT_InstaMATGraphLibraryPreviewWidget_h*/