/**
 * InstaMATSegmentedControl.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSegmentedControl.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATSegementedControl.h"
#include "InstaMATUIPCH.h"

#include "InstaMATModule.h"
#include "Slate/InstaMATPluginStyle.h"
#include "Slate/InstaMATSideBarButton.h"
#include "Widgets/Layout/SScaleBox.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"

void SInstaMATSegmentedControl::Construct(const FArguments& InArgs)
{
	ActiveElementIndex = InArgs._ActiveELementIndex;
	OnActiveButtonChanged = InArgs._OnActiveButtonChanged;
	ChildSlot
	[
		SAssignNew(Layout, SVerticalBox)
	];

	uint64 Index = 0u;
	for (const FInstaMATSegmentedControlElement ElementInformation : InArgs._ElementsInformation)
	{
		Layout->AddSlot()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		.AutoHeight()
		[
			SNew(SInstaMATSideBarButton)
				.OnButtonToggled(this, &SInstaMATSegmentedControl::OnValueChanged)
				.IconHovered(ElementInformation.IconOnHover)
				.IconNormal(ElementInformation.IconOnNormal)
				.State((Index == ActiveElementIndex) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.ToolTipText(ElementInformation.ToolTipText)
		];
		Index++;
	}

}

void SInstaMATSegmentedControl::OnValueChanged(const ECheckBoxState NewState)
{
	uint64 NewActiveElementIndex = ActiveElementIndex;

	if (NewState != ECheckBoxState::Checked)
	{
		TSharedRef<SInstaMATSideBarButton> ButtonWidget = StaticCastSharedRef<SInstaMATSideBarButton>(Layout->GetSlot(ActiveElementIndex).GetWidget());
		ButtonWidget->SetState(ECheckBoxState::Checked);
		return;
	}

	for (uint64 SlotIndex = 0u; SlotIndex < Layout->NumSlots(); SlotIndex++)
	{
		TSharedRef<SInstaMATSideBarButton> ButtonWidgetAtIndex = StaticCastSharedRef<SInstaMATSideBarButton>(Layout->GetSlot(SlotIndex).GetWidget());
		if (ButtonWidgetAtIndex->IsChecked() && SlotIndex != ActiveElementIndex)
		{
			NewActiveElementIndex = SlotIndex;
			break;
		}
	}

	TSharedRef<SInstaMATSideBarButton> UnActiveButtonWidget = StaticCastSharedRef<SInstaMATSideBarButton>(Layout->GetSlot(ActiveElementIndex).GetWidget());
	UnActiveButtonWidget->SetState(ECheckBoxState::Unchecked);

	TSharedRef<SInstaMATSideBarButton> ActiveButtonWidget = StaticCastSharedRef<SInstaMATSideBarButton>(Layout->GetSlot(NewActiveElementIndex).GetWidget());
	ActiveButtonWidget->SetState(ECheckBoxState::Checked);

	ActiveElementIndex = NewActiveElementIndex;
	OnActiveButtonChanged.ExecuteIfBound(ActiveElementIndex);
}

#undef LOCTEXT_NAMESPACE
