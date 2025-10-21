/**
 * InstaMATImporterUIUtilities.cpp (InstaMAT)
 *
 * Copyright 2019-2021 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterUIUtilities.cpp
 * @copyright 2019-2021 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATImporterUIUtilities.h"
#include "InstaMATUIPCH.h"
#include "Slate/InstaMATPluginStyle.h"

#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"

static float kTagBorderRadius = 2.0f;	/**< The tags border radius. */
static float kTagBorderWidth = 0.0f;	/**< The tags border width. */

/**< The tag background colors. */
static const TArray<FSlateRoundedBoxBrush*> kTagBackgroundColor =
{
	new FSlateRoundedBoxBrush(FLinearColor(0.004f, 0.482f, 1.0f, 1.0f), kTagBorderRadius, FLinearColor(0.004f, 0.482f, 1.0f, 1.0f), kTagBorderWidth),
	new FSlateRoundedBoxBrush(FLinearColor(0.424f, 0.459f, 0.49f, 1.0f), kTagBorderRadius, FLinearColor(0.424f, 0.459f, 0.49f, 1.0f), kTagBorderWidth),
	new FSlateRoundedBoxBrush(FLinearColor(0.153f, 0.655f, 0.271f, 1.0f), kTagBorderRadius, FLinearColor(0.153f, 0.655f, 0.271f, 1.0f), kTagBorderWidth),
	new FSlateRoundedBoxBrush(FLinearColor(0.863f, 0.208f, 0.271f, 1.0f), kTagBorderRadius, FLinearColor(0.863f, 0.208f, 0.271f, 1.0f), kTagBorderWidth),
	new FSlateRoundedBoxBrush(FLinearColor(0.09f, 0.635f, 0.722f, 1.0f), kTagBorderRadius, FLinearColor(0.09f, 0.635f, 0.722f, 1.0f), kTagBorderWidth),
	new FSlateRoundedBoxBrush(FLinearColor(0.204f, 0.227f, 0.251f, 1.0f), kTagBorderRadius, FLinearColor(0.204f, 0.227f, 0.251f, 1.0f), kTagBorderWidth),
	new FSlateRoundedBoxBrush(FLinearColor(1.0f, 0.757f, 0.039f, 1.0f), kTagBorderRadius, FLinearColor(1.0f, 0.757f, 0.039f, 1.0f), kTagBorderWidth),
	new FSlateRoundedBoxBrush(FLinearColor(0.973f, 0.976f, 0.98f, 1.0f), kTagBorderRadius, FLinearColor(0.973f, 0.976f, 0.98f, 1.0f), kTagBorderWidth)
};

TSharedRef<SVerticalBox> FInstaMATImporterUIUtilities::CreateMetaDataPanel(const FString& Name, const FString& Category, const FString& Documentation, const FString& Author, const FString& URL, const FString& Version, const FString& Tags)
{
	/// the fnCreateLabelTextField lambda generates a labeled widget to show text
	const auto fnCreateLabelTextField = [](const FString& Text, const FName& StyleName) -> TSharedRef<SHorizontalBox>
	{
		const float kDefaultPadding = 5.0f;

		return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.Padding(kDefaultPadding)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(FText::FromString(Text))
				.TextStyle(FInstaMATPluginStyle::Get(), StyleName)
			];
	};

	return SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			fnCreateLabelTextField(Name, TEXT("InstaMAT.MetaData.Text.Name"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			fnCreateLabelTextField(Category, TEXT("InstaMAT.MetaData.Text.Category"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			fnCreateLabelTextField(Documentation, TEXT("InstaMAT.MetaData.Text.Documentation"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			fnCreateLabelTextField(Author, TEXT("InstaMAT.MetaData.Text.About"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			fnCreateLabelTextField(URL, TEXT("InstaMAT.MetaData.Text.About"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			fnCreateLabelTextField(FString::Printf(TEXT("Version: %s"), *Version), TEXT("InstaMAT.MetaData.Text.About"))
		]
		+FInstaMATImporterUIUtilities::CreateTags(Tags);
}

FLinearColor FInstaMATImporterUIUtilities::GetTextColorForBackgroundColor(const FSlateRoundedBoxBrush* const BackgroundColor)
{
	static const float kTextLuminosityBright = 0.9f;
	static const float kTextLuminosityDark = 1.0f - kTextLuminosityBright;
	static const float kLuminosityThreshold = 0.45f;

	static const float kLuminosityWeightRed = 0.30f;
	static const float kLuminosityWeightGreen = 0.59f;
	static const float kLuminosityWeightBlue = 0.11f;

	const FLinearColor& Color = BackgroundColor->TintColor.GetSpecifiedColor();

	// compute the weighted luminosity of the background color
	const float BackgroundLuminosity = FMath::Clamp(Color.R * kLuminosityWeightRed + Color.G * kLuminosityWeightGreen + Color.B * kLuminosityWeightBlue, 0.0f, 1.0f);

	// for bright background colors, returns a low-luminosity text color and vice versa
	const float TextLuminosity = BackgroundLuminosity > kLuminosityThreshold ? kTextLuminosityDark : kTextLuminosityBright;

	return FLinearColor(TextLuminosity, TextLuminosity, TextLuminosity, 1.0f);
}

SVerticalBox::FSlot::FSlotArguments FInstaMATImporterUIUtilities::CreateTags(const FString& Tags)
{
	if (Tags.IsEmpty())
		return SVerticalBox::Slot();

	const float kTagBigPadding = 5.0f;
	const float kTagSmallPadding = 2.0f;

	TArray<FString> TagTokens;
	Tags.ParseIntoArray(TagTokens, TEXT(" "), /*bCullEmpty:*/ true);

	int Index = 0;
	SVerticalBox::FSlot::FSlotArguments Slot = SVerticalBox::Slot();
	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	Slot
	.AutoHeight()
	[
		HorizontalBox
	];

	for (const FString& Tag : TagTokens)
	{
		FSlateRoundedBoxBrush* const BackgroundColorBrush = kTagBackgroundColor[Index % kTagBackgroundColor.Num()];

		HorizontalBox->AddSlot()
			.AutoWidth()
			.Padding(kTagBigPadding, kTagBigPadding, kTagSmallPadding, kTagBigPadding)
			[
				SNew(SBorder)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(kTagBigPadding, 0.0f, kTagBigPadding, 0.0f)
					.BorderImage(BackgroundColorBrush)
					[
						SNew(STextBlock)
							.Text(FText::FromString(Tag))
							.ColorAndOpacity(FInstaMATImporterUIUtilities::GetTextColorForBackgroundColor(BackgroundColorBrush))
					]
			];

		Index++;
	}

	return Slot;
}