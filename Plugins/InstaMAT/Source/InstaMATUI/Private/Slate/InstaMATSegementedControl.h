/**
 * InstaMATSegmentedControl.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSegmentedControl.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

/**
 * This struct contains all information needed to add an element in the segmented control widget.
 */
struct FInstaMATSegmentedControlElement
{
	FInstaMATSegmentedControlElement(const FSlateBrush* IconOnNormal, const FSlateBrush* IconOnHover, const FText ToolTipText):
	IconOnNormal(IconOnNormal),
	IconOnHover(IconOnHover),
	ToolTipText(ToolTipText)
	{
		check(IconOnNormal != nullptr);
		check(IconOnHover != nullptr);
	};
	const FSlateBrush* IconOnNormal;		/**< The button icon. */
	const FSlateBrush* IconOnHover;			/**< The button icon when hovered. */
	const FText ToolTipText;				/**< The button tool tip text. */
};

/**
 * This class implements a segmented control for the SInstaMATSideBarButton class.
 */
class SInstaMATSegmentedControl : public SCompoundWidget
{
	DECLARE_DELEGATE_OneParam(FOnActiveButtonChanged, const uint64 /*ElementIndex*/);

	SLATE_BEGIN_ARGS(SInstaMATSegmentedControl)
	{}
	SLATE_EVENT(FOnActiveButtonChanged, OnActiveButtonChanged)
	SLATE_ARGUMENT(TArray<FInstaMATSegmentedControlElement>, ElementsInformation)
	SLATE_ARGUMENT(uint64, ActiveELementIndex)
	SLATE_END_ARGS()

public:
	/**
	 * Constructs the view of this instance.
	 *
	 * @param InArgs the construction arguments.
	 */
	void Construct(const FArguments& InArgs);

private:
	/**
	 * On button value changed callback.
	 * 
	 * @param NewState The button new state.
	 */
	void OnValueChanged(const ECheckBoxState NewState);

	FOnActiveButtonChanged OnActiveButtonChanged;		/**< Active button changed callback. */
	TSharedPtr<SVerticalBox> Layout;					/**< The internal layout. */
	uint64 ActiveElementIndex;							/**< The index of the active element. */
};
