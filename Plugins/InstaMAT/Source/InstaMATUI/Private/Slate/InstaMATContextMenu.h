/**
 * InstaMATContextMenu.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATContextMenu.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once 

#include "Widgets/SCompoundWidget.h"

/**
 * The SInstaMATSettingsWindow class provides the base
 * for context menus.
 */
class SInstaMATContextMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInstaMATContextMenu) {}
	SLATE_END_ARGS()
 
	SInstaMATContextMenu();
	~SInstaMATContextMenu();

	/**
	 * Constructs the view of this instance.
	 *
	 * @param InArgs the construction arguments.
	 */
	void Construct(const FArguments& InArgs);

	/**
	 * Adds a menu entry to the context menu.
	 * 
	 * @param Icon The Icon.
	 * @param Label The text label.
	 * @param ClickDelegate The click handler for the entry.
	 */
	void AddEntry(const FSlateBrush* const Icon, const FText& Label, FOnClicked&& ClickDelegate);

	/**
	 * Gets the widget padding.
	 * 
	 * @return The padding.
	 */
	static inline float GetWidgetPadding()
	{
		return 10.0f;
	}

	/**
	 * Gets the content padding.
	 *
	 * @return The padding.
	 */
	static inline float GetContentPadding()
	{
		return 5.0f;
	}

	/**
	 * Gets the text width.
	 *
	 * @return The width.
	 */
	static inline float GetTextWidth()
	{
		return 200.0f;
	}

	/**
	 * Gets the text height.
	 *
	 * @return The height.
	 */
	static inline float GetTextHeight()
	{
		return 40.0f;
	}

	/**
	 * Gets the icon size.
	 *
	 * @return The size.
	 */
	static inline FVector2D GetIconSize()
	{
		return FVector2D(24.0, 24.0);
	}

	/**
	 * Gets the approximate width of this widget.
	 *
	 * @return The width.
	 */
	static inline float GetApproximateWidth()
	{
		return GetIconSize().X + GetTextWidth() + GetContentPadding() * 2.0f + GetWidgetPadding() * 5.0f;
	}

private:

	TSharedPtr<SVerticalBox> MainLayout;					/**< The main layout for the context menu items. */
}; 
