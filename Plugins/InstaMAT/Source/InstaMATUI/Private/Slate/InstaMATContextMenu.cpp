/**
 * InstaMATContextMenu.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATContextMenu.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATContextMenu.h"
#include "InstaMATUIPCH.h"

#include "Widgets/Layout/SScaleBox.h"
#include "Slate/InstaMATPluginStyle.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"

SInstaMATContextMenu::SInstaMATContextMenu() : SCompoundWidget()
{
}

SInstaMATContextMenu::~SInstaMATContextMenu()
{
}

void SInstaMATContextMenu::Construct(const FArguments& InArgs)
{
	ChildSlot
	[ 
		SNew(SOverlay)
		+SOverlay::Slot()
		[
			// Background
			SNew(SImage)
			.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.ContextMenu.Background")))
		]
		+SOverlay::Slot()
		[ 
			SAssignNew(MainLayout, SVerticalBox)
		] 
	];
}

void SInstaMATContextMenu::AddEntry(const FSlateBrush* const Icon, const FText& Label, FOnClicked&& OnClickedHandler)
{
	const FVector2D kIconSize = SInstaMATContextMenu::GetIconSize();
	const float kWidgetPadding = SInstaMATContextMenu::GetWidgetPadding();
	const float kContentPadding = SInstaMATContextMenu::GetContentPadding();
	const float kTextWidth = SInstaMATContextMenu::GetTextWidth();
	const float kTextHeight = SInstaMATContextMenu::GetTextHeight();

	MainLayout->AddSlot()
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Fill)
	.Padding(0.0f)
	[
		SNew(SButton)
		.OnClicked(MoveTemp(OnClickedHandler))
		.ButtonStyle(FInstaMATPluginStyle::Get(), TEXT("InstaMATUI.Context.Item"))
		.ContentPadding(kContentPadding)
		.Content()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(kWidgetPadding, 0.0f, 0.0f, 0.0f)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::None)
				.OverrideScreenSize(kIconSize)
				[
					SNew(SImage)
					.Image(Icon)
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			.Padding(kWidgetPadding*2.0f, 0.0f, kWidgetPadding * 2.0f, 0)
			[
				// Fixed size
				SNew(SBox)
				.WidthOverride(kTextWidth)
				.HeightOverride(kTextHeight)
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					// center vertically
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(FInstaMATPluginStyle::Get(), TEXT("InstaMAT.Bold"))
						.AutoWrapText(true)
						.Justification(ETextJustify::Left)
						.Text(Label)
					]
				]
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
