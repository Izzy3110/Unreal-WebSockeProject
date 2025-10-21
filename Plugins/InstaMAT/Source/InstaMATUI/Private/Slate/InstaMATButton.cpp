/**
 * InstaMATButton.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATButton.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATButton.h"
#include "InstaMATUIPCH.h"

#include "InstaMATModule.h"
#include "Slate/InstaMATPluginStyle.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"

SInstaMATButton::SInstaMATButton() : SButton()
{
}

SInstaMATButton::~SInstaMATButton()
{
}

void SInstaMATButton::Construct(const FArguments& InArgs)
{
	const float kPadding = 5.0f;
	const FString ButtonText = InArgs._Text.Get();
	TWeakPtr<SInstaMATButton> Self = SharedThis(this);

	SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"));
	SetOnClicked(InArgs._OnClicked);
	SetVisibility(InArgs._Visibility);

	Release();

	ChildSlot
	.Padding(2.0f)
	[
		SNew(SBorder)
		.Padding(kPadding * 2.0f, kPadding)
		.BorderImage_Lambda([Self]() -> const FSlateBrush* 
		{
			if (!Self.IsValid())
				return nullptr;

			const TSharedPtr SharedObject = Self.Pin();

			if (SharedObject->IsPressed())
				return FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Button.Border.White.Pressed"));

			return SharedObject->IsHovered() ? FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Button.Border.White.Hover")) : FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Button.Border.White"));
		})
		[
			SNew(STextBlock)
			.Text(FText::FromString(ButtonText))
			.TextStyle(FInstaMATPluginStyle::Get(), TEXT("InstaMATUI.ButtonPrimary.BoldTextStyle"))
			.Justification(ETextJustify::Center)
		]
	];
}
 
#undef LOCTEXT_NAMESPACE
