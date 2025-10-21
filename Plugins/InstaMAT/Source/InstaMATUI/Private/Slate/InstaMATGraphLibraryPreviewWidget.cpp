/**
 * SInstaMATGraphLibraryPreviewWidget.cpp (InstaMAT)
 *
 * Copyright 2019-2022 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file SInstaMATGraphLibraryPreviewWidget.cpp
 * @copyright 2019-2022 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATGraphLibraryPreviewWidget.h"
#include "InstaMATImporter/Public/InstaMATImporterFactory.h"
#include "InstaMAT/Public/Slate/InstaMATPluginStyle.h"
#include "Customizations/InstaMATImporterUIUtilities.h"
#include "InstaMATButton.h"
#include "InstaMATSettingsWindow.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScaleBox.h"
#include "InstaMATUIPCH.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"

static const float kRowPinImagePaddingLeft = 4.0f;							/**< The pin image padding left. */
static const FMargin kRowPadding = FMargin(0.0f, 5.0f, 0.0f, 5.0f);			/**< The row padding. */
static const FMargin kNamePadding = FMargin(5.0f, 0.0f, 25.0f, 0.0f);		/**< The name label padding. */

/**
 * The InstaMATGraphVariableUtilities namspace contains helper 
 * functions for GraphVariables.
 */
namespace InstaMATGraphVariableUtilities
{
	/**
	 * Gets the color value for the specified \p Type.
	 * 
	 * @param Type The graph variable type.
	 * @return The color.
	 */
	static FSlateColor GetColorForGraphVariableType(const InstaMAT::IGraphVariable::Type Type)
	{
		/// The fnMakeColor macro creates a FSlateColor from uint8 color values.
#define fnMakeColor(R, G, B) FSlateColor(FColor((R), (G), (B)))
		switch (Type)
		{
		case InstaMAT::IGraphVariable::Type::TypeFloat32: return fnMakeColor(255, 0, 187);
		case InstaMAT::IGraphVariable::Type::TypeInt32: return fnMakeColor(0, 51, 255);
		case InstaMAT::IGraphVariable::Type::TypeUInt32: return fnMakeColor(0, 166, 166);
		case InstaMAT::IGraphVariable::Type::TypeVector2F: return fnMakeColor(255, 191, 255);
		case InstaMAT::IGraphVariable::Type::TypeVector3F: return fnMakeColor(187, 128, 255);
		case InstaMAT::IGraphVariable::Type::TypeVector4F: return fnMakeColor(187, 0, 255);
		case InstaMAT::IGraphVariable::Type::TypeAtomInputImage: return fnMakeColor(159, 217, 108);
		case InstaMAT::IGraphVariable::Type::TypeAtomOutputImage:
		case InstaMAT::IGraphVariable::Type::TypeElementImage: return fnMakeColor(85, 255, 0);
		case InstaMAT::IGraphVariable::Type::TypeElementMesh: return fnMakeColor(153, 128, 115);
		case InstaMAT::IGraphVariable::Type::TypeBoolean: return fnMakeColor(255, 221, 0);
		case InstaMAT::IGraphVariable::Type::TypeAtomInputImageGray: return fnMakeColor(143, 143, 143);
		case InstaMAT::IGraphVariable::Type::TypeAtomOutputImageGray:
		case InstaMAT::IGraphVariable::Type::TypeElementImageGray: return fnMakeColor(79, 79, 79);
		case InstaMAT::IGraphVariable::Type::TypeElementResource: return fnMakeColor(255, 170, 128);
		case InstaMAT::IGraphVariable::Type::TypeVector2I32: return fnMakeColor(0, 99, 166);
		case InstaMAT::IGraphVariable::Type::TypeVector3I32: return fnMakeColor(0, 119, 255);
		case InstaMAT::IGraphVariable::Type::TypeVector4I32: return fnMakeColor(38, 46, 153);
		case InstaMAT::IGraphVariable::Type::TypeVector2UI32: return fnMakeColor(0, 187, 255);
		case InstaMAT::IGraphVariable::Type::TypeVector3UI32: return fnMakeColor(0, 166, 166);
		case InstaMAT::IGraphVariable::Type::TypeVector4UI32: return fnMakeColor(0, 204, 204);
		case InstaMAT::IGraphVariable::Type::TypeMatrix2F: return fnMakeColor(255, 153, 0);
		case InstaMAT::IGraphVariable::Type::TypeMatrix3F: return fnMakeColor(166, 122, 0);
		case InstaMAT::IGraphVariable::Type::TypeMatrix4F: return fnMakeColor(166, 77, 0);
		case InstaMAT::IGraphVariable::Type::TypeElementString: return fnMakeColor(255, 85, 0);
		case InstaMAT::IGraphVariable::Type::TypeEnumValue: return fnMakeColor(255, 0, 119);
		case InstaMAT::IGraphVariable::Type::TypeElementPointCloud: return fnMakeColor(179, 89, 131);
		case InstaMAT::IGraphVariable::Type::TypeElementScene: return fnMakeColor(38, 153, 107);
		case InstaMAT::IGraphVariable::Type::TypeInvalid:
		case InstaMAT::IGraphVariable::Type::TypeOverload: return fnMakeColor(255, 0, 0);
		}
#undef fnMakeColor

		return FSlateColor(FLinearColor::Black);
	}
}

SInstaMATGraphLibraryPreviewWidget::SInstaMATGraphLibraryPreviewWidget() : 
SCompoundWidget(),
PreviewItem(nullptr)
{
	BorderBrush = MakeShareable(new FSlateColorBrush(FLinearColor(0.005f, 0.005f, 0.005f)));
}

SInstaMATGraphLibraryPreviewWidget::~SInstaMATGraphLibraryPreviewWidget()
{
}

void SInstaMATGraphLibraryPreviewWidget::SetPreviewItem(TSharedPtr<FInstaMATGraphObjectViewItem>& NewPreviewItem)
{
	PreviewItem = NewPreviewItem;
	ChildSlot.DetachWidget();
	Construct(FArguments());
}

TSharedPtr<FInstaMATGraphObjectViewItem> SInstaMATGraphLibraryPreviewWidget::GetPreviewItem()
{
	return PreviewItem;
}

TSharedRef<SHorizontalBox> SInstaMATGraphLibraryPreviewWidget::CreateGraphInputInformationRow(const FInstaMATGraphObjectInputData& Value, const float NameLabelWidth)
{
	const FVector2D kDefaultColorPreviewSize(18.0, 18.0);

	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);
	Box->SetClipping(EWidgetClipping::ClipToBoundsAlways);

	// Pin image
	Box->AddSlot()
		.Padding(kRowPinImagePaddingLeft, 0.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Input")))
			.ColorAndOpacity(InstaMATGraphVariableUtilities::GetColorForGraphVariableType(Value.Type))
			.ToolTipText(FText::FromString(Value.TypeString))
		];

	const FString InputToolTipText = Value.Documentation.Len() > 0 ? FString::Format(TEXT("{0}:\n{1}"), { Value.Name, Value.Documentation }): Value.Name;
	// Input name
	Box->AddSlot()
		.Padding(kNamePadding)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(NameLabelWidth)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Clipping(EWidgetClipping::ClipToBounds)
				.AutoWrapText(true)
				.Text(FText::FromString(Value.Name))
				.ToolTipText(FText::FromString(InputToolTipText))
			]
		];

	TSharedPtr<SWidget> InputWidget;
	static const FString kDefaultValueText = FString(TEXT("Default Value"));
	FLinearColor Color;

	const double kCornerRadius = 3.0;
	switch (Value.Type)
	{
	case InstaMAT::IGraphVariable::TypeAtomInputImage:
	case InstaMAT::IGraphVariable::TypeElementImage:
		Color = FLinearColor(Value.DefaultValue.Matrix4FValue[0], Value.DefaultValue.Matrix4FValue[1], Value.DefaultValue.Matrix4FValue[2], Value.DefaultValue.Matrix4FValue[3]);
		SAssignNew(InputWidget, SColorBlock)
			.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Separate)
			.ShowBackgroundForAlpha(true)
			.AlphaBackgroundBrush(FAppStyle::Get().GetBrush("ColorPicker.RoundedAlphaBackground"))
			.CornerRadius(FVector4(kCornerRadius, kCornerRadius, kCornerRadius, kCornerRadius))
			.Size(kDefaultColorPreviewSize)
			.Color(Color);
		break;
	case InstaMAT::IGraphVariable::TypeAtomInputImageGray:
	case InstaMAT::IGraphVariable::TypeElementImageGray:
		Color = FLinearColor(Value.DefaultValue.Matrix4FValue[0], Value.DefaultValue.Matrix4FValue[0], Value.DefaultValue.Matrix4FValue[0], 1.0f);
		SAssignNew(InputWidget, SColorBlock)
			.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Separate)
			.ShowBackgroundForAlpha(true)
			.AlphaBackgroundBrush(FAppStyle::Get().GetBrush("ColorPicker.RoundedAlphaBackground"))
			.CornerRadius(FVector4(kCornerRadius, kCornerRadius, kCornerRadius, kCornerRadius))
			.Size(kDefaultColorPreviewSize)
			.Color(Color);
		break;
	case InstaMAT::IGraphVariable::TypeBoolean:
		SAssignNew(InputWidget, SImage)
			.Image(Value.DefaultValue.BooleanValue ? FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Checked")) : FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Unchecked")));
		break;
	case InstaMAT::IGraphVariable::TypeFloat32:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s"), *FString::SanitizeFloat(Value.DefaultValue.Float32Value))));
		break;
	case InstaMAT::IGraphVariable::TypeVector2F:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("X: %s Y: %s"),
				*FString::SanitizeFloat(Value.DefaultValue.Vector2FValue[0]),
				*FString::SanitizeFloat(Value.DefaultValue.Vector2FValue[1]))));
		break;
	case InstaMAT::IGraphVariable::TypeVector3F:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("X: %s Y: %s Z: %s"),
				*FString::SanitizeFloat(Value.DefaultValue.Vector3FValue[0]),
				*FString::SanitizeFloat(Value.DefaultValue.Vector3FValue[1]),
				*FString::SanitizeFloat(Value.DefaultValue.Vector3FValue[2]))));
		break;
	case InstaMAT::IGraphVariable::TypeVector4F:

		if (Value.ControlType == InstaMAT::IGraphVariable::UIControlType::UIControlTypeColorPicker)
		{
			Color = FLinearColor(Value.DefaultValue.Vector4FValue[0], Value.DefaultValue.Vector4FValue[1], Value.DefaultValue.Vector4FValue[2], Value.DefaultValue.Vector4FValue[3]);
			SAssignNew(InputWidget, SColorBlock)
				.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Separate)
				.ShowBackgroundForAlpha(true)
				.AlphaBackgroundBrush(FAppStyle::Get().GetBrush("ColorPicker.RoundedAlphaBackground"))
				.CornerRadius(FVector4(kCornerRadius, kCornerRadius, kCornerRadius, kCornerRadius))
				.Size(kDefaultColorPreviewSize)
				.Color(Color);
		}
		else
		{
			SAssignNew(InputWidget, STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("X: %s Y: %s Z: %s W: %s"),
				*FString::SanitizeFloat(Value.DefaultValue.Vector4FValue[0]),
				*FString::SanitizeFloat(Value.DefaultValue.Vector4FValue[1]),
				*FString::SanitizeFloat(Value.DefaultValue.Vector4FValue[2]),
				*FString::SanitizeFloat(Value.DefaultValue.Vector4FValue[3]))));
		}
		break;
	case InstaMAT::IGraphVariable::TypeInt32:
	case InstaMAT::IGraphVariable::TypeUInt32:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%i"), Value.DefaultValue.Vector4I32Value[0])));
		break;
	case InstaMAT::IGraphVariable::TypeVector2I32:
	case InstaMAT::IGraphVariable::TypeVector2UI32:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("X: %i Y: %i"), Value.DefaultValue.Vector4I32Value[0], Value.DefaultValue.Vector4I32Value[1])));
		break;
	case InstaMAT::IGraphVariable::TypeVector3I32:
	case InstaMAT::IGraphVariable::TypeVector3UI32:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("X: %i Y: %i Z: %i"), Value.DefaultValue.Vector4I32Value[0], Value.DefaultValue.Vector4I32Value[1], Value.DefaultValue.Vector4I32Value[2])));
		break;
	case InstaMAT::IGraphVariable::TypeVector4I32:
	case InstaMAT::IGraphVariable::TypeVector4UI32:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("X: %i Y: %i Z: %i W: %i"), Value.DefaultValue.Vector4I32Value[0], Value.DefaultValue.Vector4I32Value[1], Value.DefaultValue.Vector4I32Value[2], Value.DefaultValue.Vector4I32Value[3])));
		break;
	case InstaMAT::IGraphVariable::TypeEnumValue:
	case InstaMAT::IGraphVariable::TypeElementString:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(Value.StringDefaultValue));
		break;
	case InstaMAT::IGraphVariable::TypeElementMesh:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(TEXT("Input Mesh")));
		break;
	case InstaMAT::IGraphVariable::TypeMatrix2F:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s, %s\n%s, %s"),
				*FString::SanitizeFloat(Value.DefaultValue.Matrix2FValue[0]),
				*FString::SanitizeFloat(Value.DefaultValue.Matrix2FValue[1]),
				*FString::SanitizeFloat(Value.DefaultValue.Matrix2FValue[2]),
				*FString::SanitizeFloat(Value.DefaultValue.Matrix2FValue[3]))));
		break;
	case InstaMAT::IGraphVariable::TypeMatrix3F:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s, %s, %s\n%s, %s, %s\n%s, %s, %s"),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[0]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[1]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[2]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[3]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[4]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[5]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[6]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[7]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix3FValue[8]))));
		break;
	case InstaMAT::IGraphVariable::TypeMatrix4F:
		SAssignNew(InputWidget, STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s, %s, %s, %s\n%s, %s, %s, %s\n%s, %s, %s, %s\n%s, %s, %s, %s"),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[0]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[1]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[2]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[3]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[4]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[5]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[6]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[7]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[8]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[9]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[10]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[11]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[12]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[13]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[14]),
			*FString::SanitizeFloat(Value.DefaultValue.Matrix4FValue[15]))));
		break;
	default:
		SAssignNew(InputWidget, STextBlock)
			.Text(FInstaMATImporterUIUtilities::gInputNotSupportedText);
		break;
	}

	if (Value.Type != InstaMAT::IGraphVariable::TypeBoolean)
	{
		InputWidget->SetToolTipText(FText::FromString(kDefaultValueText));
	}
	else
	{
		InputWidget->SetToolTipText(FText::FromString(Value.DefaultValue.BooleanValue ? TEXT("True") : TEXT("False")));
	}
	
	const float kDefaultValueWidth = 130.0f;

	Box->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				InputWidget.ToSharedRef()
			]
		];
	return Box;
}

TSharedRef<SHorizontalBox> SInstaMATGraphLibraryPreviewWidget::CreateGraphOutputInformationRow(const FInstaMATGraphObjectOutputData& Value, const float NameLabelWidth)
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);
	Box->SetClipping(EWidgetClipping::ClipToBoundsAlways);

	// Pin image
	Box->AddSlot()
		.Padding(kRowPinImagePaddingLeft, 0.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Input")))
			.ColorAndOpacity(InstaMATGraphVariableUtilities::GetColorForGraphVariableType(Value.Type))
			.ToolTipText(FText::FromString(Value.TypeString))
		];

	const FString OutputToolTipText = Value.Documentation.Len() > 0 ? FString::Format(TEXT("{0}:\n{1}"), { Value.Name, Value.Documentation }) : Value.Name;
	// Output name
	Box->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.AutoWidth()
		.Padding(kNamePadding)
		[
			SNew(SBox)
			.WidthOverride(NameLabelWidth)
			[
				SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(Value.Name))
					.ToolTipText(FText::FromString(OutputToolTipText))
			]
		];

	/// The fnColorSpaceToString lambda creates a string for color spaces.
	const auto fnColorSpaceToString = [](const InstaMAT::IGraphVariable::ColorSpaceType ColorSpaceType)
	{
		switch (ColorSpaceType)
		{
		case InstaMAT::IGraphVariable::ColorSpaceType::ColorSpaceTypeSRGB:
			return FString(TEXT("SRGB"));
		case InstaMAT::IGraphVariable::ColorSpaceType::ColorSpaceTypeLinear:
			return FString(TEXT("Linear"));
		case InstaMAT::IGraphVariable::ColorSpaceType::ColorSpaceTypeAuto:
			return FString(TEXT("Auto"));
		default:
			break;
		}
		return FString();
	};

	if (Value.Type == InstaMAT::IGraphVariable::Type::TypeAtomOutputImage ||
		Value.Type == InstaMAT::IGraphVariable::Type::TypeAtomOutputImageGray ||
		Value.Type == InstaMAT::IGraphVariable::Type::TypeElementImage ||
		Value.Type == InstaMAT::IGraphVariable::Type::TypeElementImageGray)
	{
		Box->AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.AutoWidth()
			[
				SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(fnColorSpaceToString(Value.ColorSpace)))
					.ToolTipText(FText::FromString(Value.TypeString))
			];
	}
	else
	{
		Box->AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.AutoWidth()
			[
				SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(Value.TypeString))
					.ToolTipText(FText::FromString(Value.TypeString))
			];
	}

	return Box;
}

void SInstaMATGraphLibraryPreviewWidget::Construct(const FArguments& InArgs)
{
	TWeakPtr<SInstaMATGraphLibraryPreviewWidget> WeakObject = SharedThis(this);

	const FSlateFontInfo& FontInformation = FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText").Font;
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	float MaximumLabelWidth = 0.0f;

	static const float kPadding = 5.0f;

	FInstaMATModule& InstaMATModule = FModuleManager::Get().LoadModuleChecked<FInstaMATModule>("InstaMAT");

	if (PreviewItem != nullptr)
	{
		// Force update input definitions
		InstaMATModule.GetInstaMATInterface()->GetInputAndOutputParameterDefinitions(PreviewItem->GraphID, PreviewItem->InputDefinitions, PreviewItem->OutputDefinitions);
		for (const FInstaMATGraphObjectInputData& Input : PreviewItem->InputDefinitions)
		{
			MaximumLabelWidth = FMath::Max(FontMeasure->Measure(Input.Name, FontInformation).X, MaximumLabelWidth);
		}

		for (const FInstaMATGraphObjectOutputData& Output : PreviewItem->OutputDefinitions)
		{
			MaximumLabelWidth = FMath::Max(FontMeasure->Measure(Output.Name, FontInformation).X, MaximumLabelWidth);
		}
	}

	/// The fnCreateGraphInstance lambda creates an instance of the currently selected preview graph.
	const auto fnCreateGraphInstance = [WeakObject]()
	{
		if (!WeakObject.IsValid())
			return FReply::Handled();

		const TSharedPtr SharedObject = WeakObject.Pin();

		if (SharedObject->PreviewItem == nullptr)
			return FReply::Handled();

		// If the object has no graph object, happens after a restart as the assignment is missing
		UInstaMATImporterFactory::ImportGraphObjectWithID(SharedObject->PreviewItem->GraphID);
		return FReply::Handled();
	};

	/// The fnIsCreateInstanceButtonVisible lambda determines whether the button should be visible or not. 
	const auto fnIsCreateInstanceButtonVisible = [WeakObject]()
	{
		if (!WeakObject.IsValid())
			return EVisibility::Hidden;

		const TSharedPtr SharedObject = WeakObject.Pin();
		return SharedObject->PreviewItem != nullptr ? EVisibility::Visible : EVisibility::Hidden;
	};

	/// The fnLoadPreviewImage lambda invokes the load image function.
	const auto fnLoadPreviewImage = [WeakObject]()
	{
		if (!WeakObject.IsValid())
			return FReply::Handled();

		const TSharedPtr SharedObject = WeakObject.Pin();
		FInstaMATImporterUtility::LoadPreviewTexture(*(SharedObject->PreviewItem.Get()));
		return FReply::Handled();
	};

	/// The fnIsPreviewImageVisible lambda determines whether the preview image is visible.
	const auto fnIsPreviewImageVisible = [WeakObject]()
	{
		if (!WeakObject.IsValid())
			return EVisibility::Collapsed;

		const TSharedPtr SharedObject = WeakObject.Pin();
		return SharedObject->PreviewItem != nullptr ? EVisibility::Visible : EVisibility::Collapsed;
	};

	/// The fnLoadPreviewImageButtonVisible lambda determines whether the load image preview button is visible.
	const auto fnLoadPreviewImageButtonVisible = [WeakObject] () -> EVisibility
	{
		if (!WeakObject.IsValid())
			return EVisibility::Collapsed;

		const TSharedPtr SharedObject = WeakObject.Pin();
		return	SharedObject->PreviewItem != nullptr &&
				SharedObject->PreviewItem->Preview == nullptr &&
				!FInstaMATImporterUtility::IsCachedPreviewImageAvailable(SharedObject->PreviewItem->GraphID) ? EVisibility::Visible : EVisibility::Collapsed;
	};

	/// The fnIsInstaMATLogoVisible lambda determines whether the big InstaMAT Icon is visible.
	const auto fnIsInstaMATLogoVisible = [WeakObject]() -> EVisibility
	{
		if (!WeakObject.IsValid())
			return EVisibility::Visible;

		const TSharedPtr SharedObject = WeakObject.Pin();

		if (SharedObject->PreviewItem == nullptr ||
			SharedObject->PreviewItem->Preview == nullptr)
			return EVisibility::Visible;

		if (SharedObject->PreviewItem != nullptr &&
			SharedObject->PreviewItem->Preview != nullptr)
			return EVisibility::Collapsed;
		
		return !FInstaMATImporterUtility::IsCachedPreviewImageAvailable(SharedObject->PreviewItem->GraphID) ? EVisibility::Visible : EVisibility::Collapsed;
	};

	/// The fnIsPreviewImageButtonEnabled lambda determines whether the preview button can be clicked depending on the current execution state.
	const auto fnIsPreviewImageButtonEnabled = []()
	{
		FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
		IInstaMAT* const InstaMATInterface = InstaMATModule.GetInstaMATInterface();
		return !InstaMATInterface->IsAsyncOperationInProgress();
	};

	/// The fnCreateInputViews lambda returns a view showing information about the input.
	const auto fnCreateInputViews = [WeakObject, /*copy:*/MaximumLabelWidth](TSharedRef<SScrollBox> ScrollBox)
	{
		if (!WeakObject.IsValid())
			return ScrollBox;

		const TSharedPtr SharedObject = WeakObject.Pin();
		const TSharedPtr<FInstaMATGraphObjectViewItem> Item = SharedObject->GetPreviewItem();

		if (Item == nullptr || Item->InputDefinitions.Num() == 0)
			return ScrollBox;

		const TArray<FInstaMATGraphObjectInputData>& Inputs = Item->InputDefinitions;
		TMap<FString, TArray<FInstaMATGraphObjectInputData>> CategoryToInput;
		
		for (const FInstaMATGraphObjectInputData& Input : Inputs)
		{
			const FString Category = Input.Category.IsEmpty() ? FString(TEXT("Input")) : Input.Category;

			if (!CategoryToInput.Contains(Category))
			{
				CategoryToInput.Add(Category).Add(Input);
				continue;
			}

			CategoryToInput[Category].Add(Input);
		}

		// Draw for each category all inputs
		FSlateColorBrush* const Brush = WeakObject.Pin().Get()->BorderBrush.Get();
		TSharedRef<SBorder> Border = SNew(SBorder);
		Border->SetBorderBackgroundColor(FSlateColor(FLinearColor(0.001f, 0.001f, 0.001f)));

		TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
		ScrollBox->SetOrientation(EOrientation::Orient_Vertical);

		/// The fnCreateCollapsiblePanel lambda creates a collapsible panel with the inputs.
		const auto fnCreateCollapsiblePanel = [/*copy:*/MaximumLabelWidth](const FString& Title, const TArray<FInstaMATGraphObjectInputData>& InputData, const TSharedRef<SVerticalBox>& Box)
		{
			TSharedPtr<SVerticalBox> VerticalBox = SNew(SVerticalBox);

			TSharedPtr<SBorder> BorderContainer = SNew(SBorder)
			.BorderBackgroundColor(FSlateColor(FLinearColor::Black))
			.Padding(kPadding, kPadding*2.0f, 0.0f, kPadding)
			.HAlign(HAlign_Fill)
			[
				VerticalBox.ToSharedRef()
			];

			for (const FInstaMATGraphObjectInputData& Value : InputData)
			{
				VerticalBox->AddSlot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				.AutoHeight()
				.Padding(kRowPadding)
				[
					SInstaMATGraphLibraryPreviewWidget::CreateGraphInputInformationRow(Value, MaximumLabelWidth)
				];
			}

			Box->AddSlot()
			.VAlign(VAlign_Top)
			.Padding(0.0f, 0.0f, 0.0f, kPadding)
			.AutoHeight()
			[
				SNew(SExpandableArea)
				.AllowAnimatedTransition(false)
				.Padding(FMargin(1.0f, 0.0f, 1.0f, 1.0f))
				.HeaderPadding(FMargin(kPadding*2.0f, kPadding*2.0f, kPadding*2.0f, kPadding))
				.AreaTitlePadding(FMargin(0.0f, 0.0f, kPadding*2.0f, 0.0f))
				.AreaTitle(FText::FromString(Title))
				.AreaTitleFont(FInstaMATPluginStyle::Get().GetFontStyle(TEXT("InstaMATUI.BoldFont")))
				.BorderImage(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Collapsible.HeaderBrush")))
				.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FExpandableAreaStyle>(TEXT("InstaMATUI.Collapsible")))
				.BodyBorderBackgroundColor(FSlateColor(FLinearColor::Black))
				.BodyContent()
				[
					BorderContainer.ToSharedRef()
				]
			];
		};

		if (Item->InputCategories.Num() == 0)
		{
			for (const auto& [Category, CurrentInputs] : CategoryToInput)
			{
				fnCreateCollapsiblePanel(Category, CurrentInputs, Box);
			}
		}
		else
		{
			for (const FString& Category : Item->InputCategories)
			{
				if (!CategoryToInput.Contains(Category))
					continue;

				const TArray<FInstaMATGraphObjectInputData>& CurrentInputs = CategoryToInput[Category];
				fnCreateCollapsiblePanel(Category, CurrentInputs, Box);
			}
		}

		ScrollBox->AddSlot()
		.Padding(kPadding, kPadding, kPadding, 0.0f)
		.HAlign(HAlign_Fill)
		[
			Box
		];

		return ScrollBox;
	};

	/// The fnCreateOutputViews lambda returns a view showing information about the output.
	const auto fnCreateOutputViews = [WeakObject, /*copy:*/MaximumLabelWidth](TSharedRef<SScrollBox> ScrollBox)
	{
		if (!WeakObject.IsValid())
			return ScrollBox;

		const TSharedPtr SharedObject = WeakObject.Pin();
		const TSharedPtr<FInstaMATGraphObjectViewItem> Item = SharedObject->GetPreviewItem();

		if (Item == nullptr || Item->OutputDefinitions.Num() == 0)
			return ScrollBox;

		/// The fnCreateCollapsiblePanel lambda creates a collapsible panel.
		const auto fnCreateCollapsiblePanel = [/*copy:*/MaximumLabelWidth](const FString& Title, const TArray<FInstaMATGraphObjectOutputData>& OutputData, const TSharedRef<SVerticalBox>& Box)
		{
			TSharedPtr<SVerticalBox> VerticalBox = SNew(SVerticalBox);

			TSharedPtr<SBorder> BorderContainer = SNew(SBorder)
				.BorderBackgroundColor(FSlateColor(FLinearColor::Black))
				.Padding(kPadding, kPadding*2.0f, 0.0f, kPadding)
				.HAlign(HAlign_Fill)
				[
					VerticalBox.ToSharedRef()
				];

			for (const FInstaMATGraphObjectOutputData& Value : OutputData)
			{
				VerticalBox->AddSlot()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Fill)
					.AutoHeight()
					.Padding(kRowPadding)
					[
						SInstaMATGraphLibraryPreviewWidget::CreateGraphOutputInformationRow(Value, MaximumLabelWidth)
					];
			}

			Box->AddSlot()
				.VAlign(VAlign_Top)
				.Padding(0.0f, 0.0f, 0.0f, kPadding)
				.AutoHeight()
				[
					SNew(SExpandableArea)
						.AllowAnimatedTransition(false)
						.Padding(FMargin(1.0f, 0.0f, 1.0f, 1.0f))
						.HeaderPadding(FMargin(kPadding * 2.0f, kPadding * 2.0f, kPadding * 2.0f, kPadding))
						.AreaTitlePadding(FMargin(0.0f, 0.0f, kPadding * 2.0f, 0.0f))
						.AreaTitle(FText::FromString(Title))
						.AreaTitleFont(FInstaMATPluginStyle::Get().GetFontStyle(TEXT("InstaMATUI.BoldFont")))
						.BorderImage(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Collapsible.HeaderBrush")))
						.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FExpandableAreaStyle>(TEXT("InstaMATUI.Collapsible")))
						.BodyBorderBackgroundColor(FSlateColor(FLinearColor::Black))
						.BodyContent()
						[
							BorderContainer.ToSharedRef()
						]
				];
		};

		TSharedRef<SBorder> Border = SNew(SBorder);
		Border->SetBorderBackgroundColor(FSlateColor(FLinearColor(0.001f, 0.001f, 0.001f)));

		TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
		ScrollBox->SetOrientation(EOrientation::Orient_Vertical);

		fnCreateCollapsiblePanel(TEXT("Outputs"), Item->OutputDefinitions, Box);

		ScrollBox->AddSlot()
		.Padding(kPadding, 0.0f, kPadding, kPadding)
		.HAlign(HAlign_Fill)
		[
			Box
		];

		return ScrollBox;
	};

	const float kMinimumSplitterSize = 256.0f;
	const FVector2D kPreviewImageSize(256.0, 256.0);

	/// The fnCreateMetaDataSlot macro creates a meta data slot text with the provided style
#define fnCreateMetaDataSlot(Variable, StyleValue)											\
	+SVerticalBox::Slot()																	\
	.Padding(kPadding, 0.0f, kPadding, 0.0f)												\
	.AutoHeight()																			\
	.HAlign(HAlign_Fill)																	\
	[																						\
		SNew(STextBlock)																	\
		.TextStyle(FInstaMATPluginStyle::Get(), StyleValue)									\
		.AutoWrapText(true)																	\
		.Text_Lambda([WeakObject]()															\
			{																				\
				if (!WeakObject.IsValid())													\
					return FText();															\
				const TSharedPtr SharedObject = WeakObject.Pin();							\
				if (SharedObject->PreviewItem == nullptr)									\
					return FText();															\
				return FText::FromString(SharedObject->PreviewItem->Variable);				\
			})																				\
	]

	// Display information about the graph
	TSharedRef<SSplitter> InfoBox = SNew(SSplitter)
		.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FSplitterStyle>(TEXT("InstaMATUI.Splitter")))
		.PhysicalSplitterHandleSize(3.0f);

	const FString Tags = PreviewItem.IsValid() ? PreviewItem->Tags : FString();

	InfoBox->AddSlot()
	.MinSize(kMinimumSplitterSize)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.VAlign(VAlign_Top)
		[
			SNew(SScrollBox)
			.Orientation(EOrientation::Orient_Vertical)
			+SScrollBox::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(0.0f, kPadding, 0.0f, 0.0f)

				fnCreateMetaDataSlot(GraphFriendlyName, TEXT("InstaMAT.MetaData.Text.Name"))
				fnCreateMetaDataSlot(Category, TEXT("InstaMAT.MetaData.Text.Category"))
				fnCreateMetaDataSlot(Documentation, TEXT("InstaMAT.MetaData.Text.Documentation"))
				fnCreateMetaDataSlot(Version, TEXT("InstaMAT.MetaData.Text.About"))
				fnCreateMetaDataSlot(Author, TEXT("InstaMAT.MetaData.Text.About"))
				fnCreateMetaDataSlot(URL, TEXT("InstaMAT.MetaData.Text.About"))
				
				+FInstaMATImporterUIUtilities::CreateTags(Tags)
				
				+SVerticalBox::Slot()
				.Padding(kPadding, 0.0f, 0.0f, 0.0f)
				.AutoHeight()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				[
					SNew(SScaleBox)
					.Stretch(EStretch::None)
					.OverrideScreenSize(kPreviewImageSize)
					[
						SNew(SImage)
						.Visibility_Lambda(fnIsInstaMATLogoVisible)
						.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.BigIcon")))
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				[
					SNew(SScaleBox)
					.Visibility_Lambda(fnIsPreviewImageVisible)
					.Stretch(EStretch::None)
					.OverrideScreenSize(kPreviewImageSize)
					[
						// The graph preview
						SNew(SImage)
						.Image_Lambda([WeakObject, /*copy:*/ kPreviewImageSize]() -> const FSlateBrush*
						{
							if (!WeakObject.IsValid())
								return nullptr;

							const TSharedPtr SharedObject = WeakObject.Pin();
							if (SharedObject->PreviewItem == nullptr)
								return nullptr;

							const TSharedPtr<FInstaMATGraphObjectViewItem> ViewItem = SharedObject->PreviewItem;

							if (ViewItem->Preview == nullptr && FInstaMATImporterUtility::IsCachedPreviewImageAvailable(ViewItem->GraphID))
							{
								FInstaMATImporterUtility::LoadPreviewTexture(*ViewItem.Get());
							}

							if (ViewItem->Preview == nullptr)
								return nullptr;

							// Ensure brush is valid
							if (ViewItem->Preview != nullptr && (ViewItem->GraphPreviewBrush == nullptr || !ViewItem->GraphPreviewBrush.IsValid()))
							{
								ViewItem->GraphPreviewBrush = MakeShared<FSlateDynamicImageBrush>(ViewItem->Preview, kPreviewImageSize, FName(TEXT("InstaMATPreview")));
							}

							return ViewItem->GraphPreviewBrush.Get();
						})
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Fill)
				.Padding(kPadding, kPadding, kPadding, kPadding)
				[
					SNew(SExpandableArea)
					.Visibility_Lambda(fnIsPreviewImageVisible)
					.AllowAnimatedTransition(false)
					.Padding(FMargin(1.0f, 0.0f, 1.0f, 1.0f))
					.HeaderPadding(FMargin(kPadding * 2.0f, kPadding * 2.0f, kPadding * 2.0f, kPadding))
					.AreaTitlePadding(FMargin(0.0f, 0.0f, kPadding * 2.0f, 0.0f))
					.AreaTitle(NSLOCTEXT(LOCTEXT_NAMESPACE, "ImportGraph", "Import Graph"))
					.AreaTitleFont(FInstaMATPluginStyle::Get().GetFontStyle(TEXT("InstaMATUI.BoldFont")))
					.BorderImage(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Collapsible.HeaderBrush")))
					.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FExpandableAreaStyle>(TEXT("InstaMATUI.Collapsible")))
					.BodyBorderBackgroundColor(FSlateColor(FLinearColor::Black))
					.BodyContent()
					[
						SNew(SBorder)
						.BorderBackgroundColor(FSlateColor(FLinearColor::Black))
						.Padding(kPadding*4)
						[
							SNew(STextBlock)
							.AutoWrapText(true)
							.Text(FText::FromString(TEXT("To create an instance import the graph, InstaMAT for Unreal Engine will generate a Graph object in the specified folder. The Graph object can be used to spawn instances.")))
						]
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Center)
				[
					SNew(SInstaMATButton)
					.Text(TEXT("Generate Preview Image"))
					.Visibility_Lambda(fnLoadPreviewImageButtonVisible)
					.IsEnabled_Lambda(fnIsPreviewImageButtonEnabled)
					.OnClicked_Lambda(fnLoadPreviewImage)
				]
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		.Padding(0.0f, 0.0f, 0.0f, /*Bottom:*/ 10.0f)
		[
			SNew(SInstaMATButton)
			.Text(TEXT("Import Graph"))
			.OnClicked_Lambda(fnCreateGraphInstance)
			.Visibility_Lambda(fnIsCreateInstanceButtonVisible)
		]
	];

#undef fnCreateMetaDataSlot

	ChildSlot
	.VAlign(VAlign_Fill)
	[
		InfoBox
	];

	TSharedRef<SVerticalBox> InfoVerticalBox = SNew(SVerticalBox);
	
	InfoBox->AddSlot()
	.Value(0.45f)
	.MinSize(kMinimumSplitterSize)
	[
		InfoVerticalBox
	];

	bool bIsMissingRows = true;

	// Add information if available
	if (PreviewItem != nullptr) 
	{
		TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);

		if (PreviewItem->InputDefinitions.Num() > 0)
		{
			InfoVerticalBox->AddSlot()
			.Padding(0.0f)
			[
				fnCreateInputViews(ScrollBox)
			];

			bIsMissingRows = false;
		}

		if (PreviewItem->OutputDefinitions.Num() > 0)
		{
			if (bIsMissingRows)
			{
				InfoVerticalBox->AddSlot()
				.Padding(0.0f)
				[
					fnCreateOutputViews(ScrollBox)
				];
			}
			else
			{
				fnCreateOutputViews(ScrollBox);
			}

			bIsMissingRows = false;
		}
	}

	if (bIsMissingRows)
	{
		// Add empty layout
		InfoVerticalBox->AddSlot()
		[
			SNew(SVerticalBox)
		];
	}

	// Footer
	InfoVerticalBox->AddSlot()
	.VAlign(EVerticalAlignment::VAlign_Bottom)
	.AutoHeight()
	.Padding(0.0f, /*Top:*/kPadding*2.0f, 0.0f, 0.0f)
	[
		SInstaMATSettingsWindow::CreateFooterTextBlock()
	];
}

#undef LOCTEXT_NAMESPACE
