/**
 * InstaMATSideBarButton.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSideBarButton.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "Widgets/Input/SButton.h"

/**
 * This class creates an InstaMAT side bar button.
 */
class SInstaMATSideBarButton : public SCompoundWidget
{
	DECLARE_DELEGATE_OneParam(FOnButtonToggled, const ECheckBoxState /*bToggleButtonState*/);

	SLATE_BEGIN_ARGS(SInstaMATSideBarButton){}
	SLATE_EVENT(FOnButtonToggled, OnButtonToggled)
	SLATE_ARGUMENT(const FSlateBrush*, IconNormal)
	SLATE_ARGUMENT(const FSlateBrush*, IconHovered)
	SLATE_ARGUMENT(ECheckBoxState, State)
	SLATE_END_ARGS()

public:
	/**
	 * Constructs the view of this instance.
	 *
	 * @param InArgs the construction arguments.
	 */
	void Construct(const FArguments& InArgs);

	/**
	* Whether the button is checked.
	* 
	* @return True if the button is checked.
	*/
	bool IsChecked() const;

	/**
	* Sets the state of the button.
	*
	* @param NewState The new state of the button.
	*/
	void SetState(const ECheckBoxState NewState);

private:
	TSharedPtr<SButton> Button;					/**< The internal button. */
	FOnButtonToggled OnButtonToggled;			/**< Button toggled callback. */
	const FSlateBrush* ButtonOnNormalIcon;		/**< The icon of the button. */
	const FSlateBrush* ButtonOnHoveredIcon;		/**< The icon of the button when hovered. */
	bool bToggleButtonState;					/**< Holds the button state. */
};
