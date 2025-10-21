/**
 * InstaMATSideBarButton.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSideBarButton.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATSideBarButton.h"
#include "InstaMATUIPCH.h"

#include "InstaMATModule.h"
#include "Slate/InstaMATPluginStyle.h"
#include "Widgets/Layout/SScaleBox.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"

void SInstaMATSideBarButton::Construct(const FArguments& InArgs)
{
	bToggleButtonState = (InArgs._State == ECheckBoxState::Checked);
	OnButtonToggled = InArgs._OnButtonToggled;
	ButtonOnNormalIcon = InArgs._IconNormal;
	ButtonOnHoveredIcon = InArgs._IconHovered;

	TWeakPtr<SInstaMATSideBarButton> Self = SharedThis(this);

	ChildSlot
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				.HeightOverride(30.0f)
				.WidthOverride(30.0f)
				[
					SNew(SImage)
					.Image_Lambda([Self]() -> const FSlateBrush*
					{
						if (!Self.IsValid())
							return FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.Background.Brush.Dark"));

						const TSharedPtr SharedObject = Self.Pin();
						if (SharedObject->bToggleButtonState)
							return FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.Background.Brush.Light"));

						return SharedObject->IsHovered()?
							FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.Background.Brush.Light")) :
							FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.Background.Brush.Dark"));
					})
				]
			]
			+SOverlay::Slot()
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "NoBorder")
				.OnClicked_Lambda([Self]() -> FReply
				{
					if (!Self.IsValid())
						return FReply::Handled();
					const TSharedPtr SharedObject = Self.Pin();

					SharedObject->bToggleButtonState = !SharedObject->bToggleButtonState;
					SharedObject->OnButtonToggled.ExecuteIfBound(SharedObject->bToggleButtonState?ECheckBoxState::Checked:ECheckBoxState::Unchecked);
					return FReply::Handled();
				})
				.Content()
				[
					SNew(SScaleBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image_Lambda([Self]() -> const FSlateBrush*
						{
							if (!Self.IsValid())
								return nullptr;

							const TSharedPtr SharedObject = Self.Pin();
							return SharedObject->IsHovered() ? SharedObject->ButtonOnHoveredIcon : SharedObject->ButtonOnNormalIcon;
						})
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			[
				SNew(SBox)
				.Visibility(EVisibility::HitTestInvisible)
				.WidthOverride(2.0f)
				[
					SNew(SImage)
					.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.BlueBorder")))
					.Visibility_Lambda([Self]()->EVisibility
					{
						if (!Self.IsValid())
							return EVisibility::Visible;

						const TSharedPtr SharedObject = Self.Pin();
						return SharedObject->bToggleButtonState ? EVisibility::Visible : EVisibility::Hidden;
					})
				]
			]
		]
	];
}

bool SInstaMATSideBarButton::IsChecked() const
{
	return bToggleButtonState;
}

void SInstaMATSideBarButton::SetState(const ECheckBoxState NewState)
{
	bToggleButtonState = (NewState == ECheckBoxState::Checked);
}

#undef LOCTEXT_NAMESPACE