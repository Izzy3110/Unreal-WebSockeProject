/**
 * InstaMATImporterGraphInstanceCustomization.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraphInstanceCustomization.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATImporterGraphInstanceCustomization.h"
#include "InstaMATUIPCH.h"

#include "Slate/InstaMATPluginStyle.h"
#include "InstaMATModule.h"
#include "InstaMAT/InstaMATEnum.h"
#include "InstaMAT/InstaMATSettings.h"

#include "InstaMATImporter/Public/InstaMATImporterFactory.h"
#include "InstaMATImporter/Public/InstaMATImporterGraphInstance.h"
#include "InstaMATImporter/Public/ParameterObjects/InstaMATInputBase.h"
#include "InstaMATImporter/Public/ParameterObjects/InstaMATOutput.h"
#include "InstaMATImporter/Public/ParameterObjects/InstaMATMeshOutput.h"
#include "InstaMATImporter/Public/ParameterObjects/InstaMATGraphSceneOutput.h"
#include "InstaMATImporterUIUtilities.h"
#include "Slate/InstaMATButton.h"

#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "SEnumCombo.h"
#include "SResetToDefaultPropertyEditor.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Dialog/SCustomDialog.h"
#include "Templates/SharedPointer.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Algo/ForEach.h"

static const double kBrushSize = 48.0; /**< Default brush size for previews. */

#define LOCTEXT_NAMESPACE "InstaMATUI"

namespace InstaMATUIHelper
{
	/**
	 * Determines whether an async progress is in process.
	 *
	 * @return True upon success.
	 */
	static bool IsAsyncOperationInProgress()
	{
		FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
		IInstaMAT* const InstaMATInterface = InstaMATModule.GetInstaMATInterface();
		return InstaMATInterface->IsAsyncOperationInProgress();
	}

	/**
	 * This function initiates the custom UI creation process.
	 *
	 * @param Setting The InstaMAT input setting object.
	 * @param DetailBuilder The detailbuilder.
	 * @param InputCategory The input category name.
	 */
	static void CreateInputGroup(UInstaMATInputBase* const Setting, IDetailCategoryBuilder& DetailBuilder, const FString& InputCategory)
	{
		check(Setting != nullptr);
		const float kPadding = 5.0f;
		const float kCornerRadius = 4.0f;

		TWeakObjectPtr<UInstaMATInputBase> WeakObject = Setting;

		// The UIElementVisibleAttribute Attribute is determining the visibility state of the UI elements.
		TAttribute<EVisibility> UIElementVisibleAttribute;
		UIElementVisibleAttribute.Bind(TDelegate<EVisibility()>::CreateLambda([WeakObject]()
		{
			if (!WeakObject.IsValid())
				return EVisibility::Collapsed;

			return WeakObject->bIsVisible ? EVisibility::Visible : EVisibility::Collapsed;
		}));

		// The UIEditConditionAttribute is determining the editable state of the UI elements
		TAttribute<bool> UIEditConditionAttribute;
		UIEditConditionAttribute.Bind(TDelegate<bool()>::CreateLambda([WeakObject]()
		{
			if (!WeakObject.IsValid())
				return false;

			return WeakObject->bIsVisible && !InstaMATUIHelper::IsAsyncOperationInProgress();
		}));

		const FText ToolTip = FText::FromString(Setting->ToolTip);
		const FCommentNodeSet Set = { Setting };
		const FString BeautifiedCategory = FName::NameToDisplayString(InputCategory, /*bIsBool:*/ false);

		FString InputName = Setting->InputName;

		// Remove category from input setting name
		if (!BeautifiedCategory.IsEmpty() && (InputName.StartsWith(BeautifiedCategory) || InputName.EndsWith(BeautifiedCategory)))
		{
			InputName.ReplaceInline(*BeautifiedCategory, TEXT(""));
			InputName = InputName.TrimStartAndEnd();

			// Revert if the resulting string is empty
			if (InputName.IsEmpty())
			{
				InputName = Setting->InputName;
			}
		}

		const FText InputNameText = FText::FromString(InputName);

		/// The fnSetDefaultParameterForRow lambda sets the default row settings.
		const auto fnSetDefaultParameterForRow = [&ToolTip, &UIElementVisibleAttribute, &UIEditConditionAttribute, &InputNameText](IDetailPropertyRow* const Row)
		{
			check(Row != nullptr);
			Row->Visibility(UIElementVisibleAttribute);
			Row->EditCondition(UIEditConditionAttribute, FOnBooleanValueChanged());
			Row->DisplayName(InputNameText);
			Row->ToolTip(ToolTip);
		};

		/// The fnSetDefaultResetSettingForRow sets the InstaMAT for Unreal Engine default reset behavior.
		const auto fnSetDefaultResetSettingForRow = [&WeakObject](IDetailPropertyRow* const Row)
		{
			check(Row != nullptr);
			const FIsResetToDefaultVisible IsResetButtonVisibleLambda = FIsResetToDefaultVisible::CreateLambda([WeakObject](TSharedPtr<IPropertyHandle> Property) -> bool {
				if (!WeakObject.IsValid())
					return false;
			
				return !WeakObject->IsDefaultValue(); 
			});

			const FResetToDefaultHandler ResetToDefaultLambda = FResetToDefaultHandler::CreateLambda([WeakObject](TSharedPtr<IPropertyHandle> Property) {
				if (!WeakObject.IsValid()) 
					return; 

				WeakObject->Reset(); 
			});

			const FResetToDefaultOverride ResetOverrideBehavior = FResetToDefaultOverride::Create(IsResetButtonVisibleLambda, ResetToDefaultLambda, /*InPropagateToChildren:*/ true);
			Row->OverrideResetToDefault(ResetOverrideBehavior);
		};

		// NOTE: need custom behavior for image input
		if (Setting->IsA<UInstaMATInputElementImage>())
		{
			// show the texture value assignment view
			IDetailPropertyRow* Row = DetailBuilder.AddExternalObjectProperty(Set, FName(TEXT("TextureValue")), EPropertyLocation::Default, FAddPropertyParams());
			fnSetDefaultParameterForRow(Row);

			// show the color value assignment view
			Row = DetailBuilder.AddExternalObjectProperty(Set, FName(TEXT("ColorValue")), EPropertyLocation::Default, FAddPropertyParams());
			fnSetDefaultParameterForRow(Row);

			UInstaMATInputElementImage* const SettingImage = Cast<UInstaMATInputElementImage>(Setting);
			TAttribute<bool> IsColorSettingEnabled;
			IsColorSettingEnabled.BindUObject(SettingImage, &UInstaMATInputElementImage::IsColorValueEnabled);
			TWeakObjectPtr<UInstaMATInputElementImage> WeakImageObject = SettingImage;

			const FIsResetToDefaultVisible IsResetButtonVisibleLambda = FIsResetToDefaultVisible::CreateLambda([WeakImageObject](TSharedPtr<IPropertyHandle> Property) { if (!WeakImageObject.IsValid()) return false; return !WeakImageObject.Get()->IsDefaultColorValue(); });
			const FResetToDefaultHandler ResetToDefaultLambda = FResetToDefaultHandler::CreateLambda([WeakImageObject](TSharedPtr<IPropertyHandle> Property) { if (!WeakImageObject.IsValid()) return; WeakImageObject.Get()->ResetColor(); });
			const FResetToDefaultOverride ResetOverrideBehavior = FResetToDefaultOverride::Create(IsResetButtonVisibleLambda, ResetToDefaultLambda, true);

			Row->IsEnabled(IsColorSettingEnabled);
			Row->OverrideResetToDefault(ResetOverrideBehavior);
		}
		else if (Setting->IsA<UInstaMATInputElementImageGrayscale>())
		{
			// show the texture value assignment view
			IDetailPropertyRow* Row = DetailBuilder.AddExternalObjectProperty(Set, FName(TEXT("TextureValue")), EPropertyLocation::Default, FAddPropertyParams());
			fnSetDefaultParameterForRow(Row);

			// show the color value assignment view
			Row = DetailBuilder.AddExternalObjectProperty(Set, FName(TEXT("GrayscaleValue")), EPropertyLocation::Default, FAddPropertyParams());
			fnSetDefaultParameterForRow(Row);

			TSharedPtr<IPropertyHandle> PropertyHandle = Row->GetPropertyHandle();

			// Clamp values to 0-1 range
			PropertyHandle->SetInstanceMetaData(FName(TEXT("ClampMin")), FString(TEXT("0.0")));
			PropertyHandle->SetInstanceMetaData(FName(TEXT("ClampMax")), FString(TEXT("1.0")));

			UInstaMATInputElementImageGrayscale* const SettingImage = Cast<UInstaMATInputElementImageGrayscale>(Setting);
			TAttribute<bool> IsGrayscaleSettingEnabled;
			IsGrayscaleSettingEnabled.BindUObject(SettingImage, &UInstaMATInputElementImageGrayscale::IsGrayscaleValueEnabled);
			TWeakObjectPtr<UInstaMATInputElementImageGrayscale> WeakImageObject = SettingImage;

			const FIsResetToDefaultVisible IsResetButtonVisibleLambda = FIsResetToDefaultVisible::CreateLambda([WeakImageObject](TSharedPtr<IPropertyHandle> Property) { if (!WeakImageObject.IsValid()) return false; return !WeakImageObject.Get()->IsDefaultGrayscaleValue(); });
			const FResetToDefaultHandler ResetToDefaultLambda = FResetToDefaultHandler::CreateLambda([WeakImageObject](TSharedPtr<IPropertyHandle> Property) { if (!WeakImageObject.IsValid()) return; WeakImageObject.Get()->ResetGrayscaleValue(); });
			const FResetToDefaultOverride ResetOverrideBehavior = FResetToDefaultOverride::Create(IsResetButtonVisibleLambda, ResetToDefaultLambda, true);

			Row->OverrideResetToDefault(ResetOverrideBehavior);

			Row->IsEnabled(IsGrayscaleSettingEnabled);
		}
		else if (Setting->IsA<UInstaMATInputEnumValue>())
		{
			UInstaMATInputEnumValue* const EnumParameter = Cast<UInstaMATInputEnumValue>(Setting);

			if (EnumParameter->EnumStringValues.Num() == 0 && EnumParameter->EnumValues.Num() == 0)
			{
				UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Enum values got lost, please consider reimporting the material and saving after creating."));
				return;
			}
			// NOTE: Shared pointers can't be used for properties.
			if (EnumParameter->EnumStringValues.Num() == 0)
			{
				for (const FString& Value : EnumParameter->EnumValues)
				{
					EnumParameter->EnumStringValues.Add(MakeShareable<FString>(new FString(Value)));
				}
			}

			TWeakObjectPtr<UInstaMATInputEnumValue> WeakEnumObject = EnumParameter;

			TSharedRef<STextComboBox> TextComboBox = SNew(STextComboBox)
				.ToolTipText(ToolTip)
				.OptionsSource(&(EnumParameter->EnumStringValues))
				.Font(FAppStyle::Get().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
				.IsEnabled_Lambda([]() -> bool { return !InstaMATUIHelper::IsAsyncOperationInProgress(); })
				.OnSelectionChanged_Lambda([WeakEnumObject](TSharedPtr<FString> Selection, ESelectInfo::Type SelectInfo)
					{
						if (!WeakEnumObject.IsValid())
							return;

						const int ArrayIndex = WeakEnumObject->EnumStringValues.Find(Selection);

						if (ArrayIndex == -1)
							return;

						WeakEnumObject->Value = ArrayIndex;
						WeakEnumObject->PostEditChange();
					})
				.InitiallySelectedItem(EnumParameter->EnumStringValues[EnumParameter->Value]);

			IDetailPropertyRow* const Row = DetailBuilder.AddExternalObjectProperty(Set, FName(TEXT("Value")), EPropertyLocation::Default, FAddPropertyParams());

			Row->Visibility(UIElementVisibleAttribute)
				.EditCondition(UIEditConditionAttribute, FOnBooleanValueChanged());

			// Initialize row widgets, the GetDefaultWidgets creates the widgets if not available
			TSharedPtr<SWidget> NameWidget;	// unused
			TSharedPtr<SWidget> ValueWidget;// unused
			Row->GetDefaultWidgets(NameWidget, ValueWidget);

			Row->CustomWidget()
				.NameContent()
				.HAlign(HAlign_Fill)
				[
					// label
					SNew(STextBlock)
						.Margin(FMargin(0.0f, kPadding, 0.0f, 0.0f))
						.Font(FAppStyle::Get().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
						.Text(InputNameText)
						.ToolTipText(ToolTip)
				]
				.ValueContent()
				.HAlign(HAlign_Left)
				[
					TextComboBox
				];
			
			TSharedPtr<STextComboBox> ComboBoxShared = TextComboBox.ToSharedPtr();
			const FIsResetToDefaultVisible IsResetButtonVisibleLambda = FIsResetToDefaultVisible::CreateLambda([WeakEnumObject](TSharedPtr<IPropertyHandle> Property) { if (!WeakEnumObject.IsValid()) return false; return !WeakEnumObject.Get()->IsDefaultValue(); });
			const FResetToDefaultHandler ResetToDefaultLambda = FResetToDefaultHandler::CreateLambda([WeakEnumObject, ComboBoxShared](TSharedPtr<IPropertyHandle> Property)
				{
					if (!WeakEnumObject.IsValid() || !ComboBoxShared.IsValid())
						return;
					
					WeakEnumObject->Reset();

					// revert control
					ComboBoxShared->SetSelectedItem(WeakEnumObject->EnumStringValues[WeakEnumObject->DefaultValue]);
				});
			const FResetToDefaultOverride ResetOverrideBehavior = FResetToDefaultOverride::Create(IsResetButtonVisibleLambda, ResetToDefaultLambda, true);
			
			Row->OverrideResetToDefault(ResetOverrideBehavior);
		}
		else if (Setting->IsA<UInstaMATInputNotSupported>())
		{
			IDetailPropertyRow* const Row = DetailBuilder.AddExternalObjectProperty(Set, FName(TEXT("")), EPropertyLocation::Default, FAddPropertyParams());
			fnSetDefaultParameterForRow(Row);

			// FIXME: Create an INSTA_UNUSED() macro.
			
			// Initialize row widgets. "GetDefaultWidgets()" creates the widgets if not yet available.
			TSharedPtr<SWidget> NameWidget;	// unused
			TSharedPtr<SWidget> ValueWidget; // unused
			Row->GetDefaultWidgets(NameWidget, ValueWidget);

			// Attribute defining the visibility of the UI elements.
			TAttribute<EVisibility> NotSupportedInputVisibleAttribute;
			NotSupportedInputVisibleAttribute.Bind(TDelegate<EVisibility()>::CreateLambda([/*copy:*/ WeakObject]()
			{
				if (!WeakObject.IsValid())
					return EVisibility::Collapsed;

				const UInstaMATSettings* const InstaMATSettings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
				check(InstaMATSettings != nullptr);

				return WeakObject->bIsVisible && InstaMATSettings->bIsUnsupportedTypesVisible ? EVisibility::Visible : EVisibility::Collapsed;
			}));

			Row->CustomWidget()
				.Visibility(NotSupportedInputVisibleAttribute)
				.NameContent()
				.HAlign(HAlign_Fill)
				[
					// Label
					SNew(STextBlock)
						.Margin(FMargin(0.0f, kPadding, 0.0f, 0.0f))
						.Font(FAppStyle::Get().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
						.Text(InputNameText)
						.ToolTipText(ToolTip)
				]
			.ValueContent()
				.HAlign(HAlign_Left)
				[
					// Value
					SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							.Padding(kPadding)
							[
								SNew(SScaleBox)
									.Stretch(EStretch::None)
									[
										SNew(SImage)
											.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Icons.Warning")))
											.DesiredSizeOverride(FVector2D(16.0f, 16.0f))
											.ToolTipText(FInstaMATImporterUIUtilities::gInputNotSupportedTextTooltip)
									]
							]
						+SHorizontalBox::Slot()
							.Padding(kPadding)
							.AutoWidth()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
									.Font(FAppStyle::Get().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
									.Text(FInstaMATImporterUIUtilities::gInputNotSupportedText)
									.ToolTipText(FInstaMATImporterUIUtilities::gInputNotSupportedTextTooltip)
							]
				];
		}
		else
		{
			// Create special UI for color picker
			if (Setting->IsA<UInstaMATInputVector4F>() && Setting->ControlType == InstaMAT::IGraphVariable::UIControlType::UIControlTypeColorPicker)
			{
				UInstaMATInputVector4F* const ColorParameter = Cast<UInstaMATInputVector4F>(Setting);
				TWeakObjectPtr<UInstaMATInputVector4F> WeakColorObject = ColorParameter;

				TSharedPtr<SBorder> Border;

				TSharedRef<SColorBlock> ColorBlock = SNew(SColorBlock)
					.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Separate)
					.ShowBackgroundForAlpha(true)
					.AlphaBackgroundBrush(FAppStyle::Get().GetBrush("ColorPicker.RoundedAlphaBackground"))
					.ToolTipText(ToolTip)
					.Size(FVector2D(70.0, 20.0))
					.CornerRadius(FVector4f(kCornerRadius, kCornerRadius, kCornerRadius, kCornerRadius))
					.OnMouseButtonDown_Lambda([WeakColorObject](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
						{
							if (!WeakColorObject.IsValid())
								return FReply::Handled();

							const FVector4f Value = WeakColorObject->Value;
							const FLinearColor Color(Value.X, Value.Y, Value.Z, Value.W);

							FColorPickerArgs Arguments;
							Arguments.bIsModal = true;
							Arguments.bUseAlpha = true;
							Arguments.bOnlyRefreshOnMouseUp = true;
							Arguments.InitialColor = Color;
							Arguments.OnColorCommitted = FOnLinearColorValueChanged::CreateLambda([WeakColorObject](FLinearColor NewColor)
								{
									if (!WeakColorObject.IsValid())
										return;

									WeakColorObject->Value = FVector4f(NewColor.R, NewColor.G, NewColor.B, NewColor.A);
									WeakColorObject->PostEditChange();
								});
							OpenColorPicker(Arguments);
							return FReply::Handled();
						})
					.Color_Lambda([WeakColorObject]() -> FColor 
						{
							if (!WeakColorObject.IsValid())
								return FColor::Black;

							const FVector4f Value = WeakColorObject->Value;
							return FLinearColor(Value.X, Value.Y, Value.Z, Value.W).ToFColor(/*bIsSRGB:*/ true);
						})
					.IsEnabled_Lambda([]() -> bool { return !InstaMATUIHelper::IsAsyncOperationInProgress(); });

				IDetailPropertyRow* const Row = DetailBuilder.AddExternalObjectProperty(Set, FName("Value"), EPropertyLocation::Default, FAddPropertyParams());

				Row->Visibility(UIElementVisibleAttribute)
					.EditCondition(UIEditConditionAttribute, FOnBooleanValueChanged());

				// Initialize row widgets, the GetDefaultWidgets creates the widgets if not available
				TSharedPtr<SWidget> NameWidget;	// unused
				TSharedPtr<SWidget> ValueWidget;// unused
				Row->GetDefaultWidgets(NameWidget, ValueWidget);

				SAssignNew(Border, SBorder)
				.Padding(1)
				.BorderImage(FAppStyle::Get().GetBrush("ColorPicker.RoundedSolidBackground"))
				.VAlign(VAlign_Center)
				[
					ColorBlock
				];

				// Background color attribute to show the correct outline color while hovering.
				const TAttribute<FSlateColor> BackgroundColorAttribute = TAttribute<FSlateColor>::Create([Border]()
					{
						static const FSlateColor HoveredColor = FAppStyle::Get().GetSlateColor("Colors.Hover");
						static const FSlateColor DefaultColor = FAppStyle::Get().GetSlateColor("Colors.InputOutline");

						if (!Border.IsValid())
							return DefaultColor;

						return Border->IsHovered() ? HoveredColor : DefaultColor;
					});
					
				Border->SetBorderBackgroundColor(BackgroundColorAttribute);

				// Add controls to widgets
				Row->CustomWidget()
				.NameContent()
				.HAlign(HAlign_Fill)
				[
					// label
					SNew(STextBlock)
						.Margin(FMargin(0.0f, kPadding, 0.0f, 0.0f))
						.Font(FAppStyle::Get().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
						.Text(InputNameText)
						.ToolTipText(ToolTip)
				]
				.ValueContent()
				.HAlign(HAlign_Left)
				[
					Border.ToSharedRef()
				];

				fnSetDefaultResetSettingForRow(Row);
			}
			else
			{
				IDetailPropertyRow* const Row = DetailBuilder.AddExternalObjectProperty(Set, FName(TEXT("Value")), EPropertyLocation::Default, FAddPropertyParams());
				fnSetDefaultParameterForRow(Row);
				
				TSharedPtr<IPropertyHandle> Handle = Row->GetPropertyHandle();

				/// The fnApplyClampValuesAsMetaData macro sets the meta data min max value
#define fnApplyClampValuesAsMetaData(type, setting, handle)																	\
	if (setting->IsA<type>())																								\
	{																														\
		type* const value = Cast<type>(setting);																			\
		if (value->bIsRangeLimited)																							\
		{																													\
			handle->SetInstanceMetaData(FName(TEXT("ClampMin")), FString::Format(TEXT("{0}"), { value->MinimumValue }));	\
			Handle->SetInstanceMetaData(FName(TEXT("ClampMax")), FString::Format(TEXT("{0}"), { value->MaximumValue }));	\
		}																													\
	}

				fnApplyClampValuesAsMetaData(UInstaMATInputFloat32, Setting, Handle);
				fnApplyClampValuesAsMetaData(UInstaMATInputInt32, Setting, Handle);

#undef fnApplyClampValuesAsMetaData

				fnSetDefaultResetSettingForRow(Row);
			}
		}
	}
};

FInstaMATImporterGraphInstanceCustomization::FInstaMATImporterGraphInstanceCustomization() : 
DetailLayoutBuilder(nullptr)
{
}

FInstaMATImporterGraphInstanceCustomization::~FInstaMATImporterGraphInstanceCustomization()
{
	DetailLayoutBuilder = nullptr;
}

void FInstaMATImporterGraphInstanceCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailLayoutBuilder = &DetailBuilder;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingEdited;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingEdited);

	if (ObjectsBeingEdited.Num() != 1)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Can only edit single object."));
		return;
	}

	TWeakObjectPtr<UObject> CurrentUObject = ObjectsBeingEdited[0];

	if (CurrentUObject == nullptr)
		return;

	UInstaMATImporterGraphInstance* const CurrentObject = Cast<UInstaMATImporterGraphInstance>(CurrentUObject);

	if (CurrentObject == nullptr)
		return;

	const FString ExecuteCategoryName = FString(TEXT("Execute"));
	TWeakObjectPtr<UInstaMATImporterGraphInstance> WeakObject = CurrentObject;

	FInstaMATModule& Module = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	IInstaMAT* const InstaMATModule = Module.GetInstaMATInterface();

	check(InstaMATModule != nullptr);

	// set edit conditions
	TAttribute<bool> AsyncOperationInProcessAttribute;
	AsyncOperationInProcessAttribute.Bind(TDelegate<bool()>::CreateLambda([]() -> bool
		{
			return !InstaMATUIHelper::IsAsyncOperationInProgress();
		}));

	TAttribute<bool> PreviewModeEnabledAttribute;
	PreviewModeEnabledAttribute.Bind(TDelegate<bool()>::CreateLambda([WeakObject]() -> bool
		{
			if (!WeakObject.IsValid())
				return false;

			return !WeakObject->bIsPreviewMode && !InstaMATUIHelper::IsAsyncOperationInProgress();
		}));

	TAttribute<EVisibility> VisibilityRotationAttribute;
	VisibilityRotationAttribute.Bind(TDelegate<EVisibility()>::CreateLambda([WeakObject]() -> EVisibility
	{
		if (!WeakObject.IsValid())
			return EVisibility::Collapsed;

		return (!InstaMATUIHelper::IsAsyncOperationInProgress() && WeakObject->OutputParameters.Num() > 0u)? EVisibility::Visible : EVisibility::Collapsed;
	}
	));

	TAttribute<bool> PhysicalSizeAttribute;
	PhysicalSizeAttribute.Bind(TDelegate<bool()>::CreateLambda([WeakObject]() -> bool
		{
			if (!WeakObject.IsValid())
				return false;

			return !WeakObject->bHasPhysicalSize;
		}));

	TAttribute<EVisibility> VisibilityPhysicalSizeAttribute;
	VisibilityPhysicalSizeAttribute.Bind(TDelegate<EVisibility()>::CreateLambda([WeakObject]() -> EVisibility
		{
			if (!WeakObject.IsValid())
				return EVisibility::Collapsed;

			return WeakObject->bHasPhysicalSize ? EVisibility::Visible : EVisibility::Collapsed;
		}));

	// set visibility conditions for progressbar
	TAttribute<EVisibility> AsyncOperationInProcessVisibilityAttribute;
	AsyncOperationInProcessVisibilityAttribute.Bind(TDelegate<EVisibility()>::CreateLambda([]() -> EVisibility
		{
			return InstaMATUIHelper::IsAsyncOperationInProgress() ? EVisibility::Visible : EVisibility::Hidden;
		}));

	TAttribute<TOptional<float>> ExecutionProgressAttribute;
	ExecutionProgressAttribute.Bind(TDelegate<TOptional<float>()>::CreateLambda([/*copy:*/InstaMATModule]() -> TOptional<float>
		{
			return TOptional<float>(*InstaMATModule->GetProgressValue());
		}));

	static const TArray<FName> DefaultPropertyNamesForVisibility = { TEXT("Seed"), TEXT("bIsPreviewMode") };
	static const TArray<FName> PreviewDependentPropertyNamesForVisibility = { TEXT("ResolutionWidth"), TEXT("ResolutionHeight"), TEXT("ExecutionFormat") };
	
	/// The fnApplyVisibilityAttributeToProperty lambda applies a visibility attribute to a property with the specified name.
	const auto fnApplyVisibilityAttributeToProperty = [&DetailBuilder](const FName& PropertyName, TAttribute<bool>& EditCondition)
	{
		TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(PropertyName);
		DetailBuilder.EditDefaultProperty(PropertyHandle)->EditCondition(EditCondition, FOnBooleanValueChanged());
	};

	const auto fnApplyDefaultVisibilityAttributeToProperty = [&fnApplyVisibilityAttributeToProperty, &AsyncOperationInProcessAttribute](const FName& PropertyName) { return fnApplyVisibilityAttributeToProperty(PropertyName, AsyncOperationInProcessAttribute); };
	const auto fnApplyPreviewModeVisibilityAttributeToProperty = [&fnApplyVisibilityAttributeToProperty, &PreviewModeEnabledAttribute](const FName& PropertyName) { return fnApplyVisibilityAttributeToProperty(PropertyName, PreviewModeEnabledAttribute); };

	Algo::ForEach(DefaultPropertyNamesForVisibility, fnApplyDefaultVisibilityAttributeToProperty);
	Algo::ForEach(PreviewDependentPropertyNamesForVisibility, fnApplyPreviewModeVisibilityAttributeToProperty);

	TSharedRef<IPropertyHandle> RotationPropertyHandle = DetailBuilder.GetProperty(TEXT("Rotation"));
	DetailBuilder.EditDefaultProperty(RotationPropertyHandle)->Visibility(VisibilityRotationAttribute);
	
	TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(TEXT("bEnablePhysicalSize"));
	DetailBuilder.EditDefaultProperty(PropertyHandle)->Visibility(VisibilityPhysicalSizeAttribute);
	
	if (!CurrentObject->bHasPhysicalSize)
	{
		DetailBuilder.HideProperty(PropertyHandle);
	}

	// build UI
	int SortOrder = 0;
	const float kPadding = 10.0f;

	if (CurrentObject->PreviewImage == nullptr)
	{
		if (FInstaMATImporterUtility::IsCachedPreviewImageAvailable(CurrentObject->GraphID))
		{
			uint32 Width;
			uint32 Height;
			TArray<FColor> PixelData;
			if (InstaMATModule->TryLoadingPreviewImageFromCache(CurrentObject->GraphID, CurrentObject->ImportFilePath, Width, Height, PixelData))
			{
				CurrentObject->PreviewImage = FInstaMATImporterUtility::CreateTextureFromBitmapData(Width, Height, PixelData);
			}
		}
		else
		{
			CurrentObject->PreviewImage = FInstaMATImporterUtility::CreatePreviewImage(Module, CurrentObject->GetPackage(), CurrentObject->ImportFilePath, FInstaMATImporterUtility::EnsureValidObjectName(FString(TEXT("Preview")) + CurrentObject->GetName()), CurrentObject->GraphID, /*bUseAlpha:*/ true);
		}
	}

	const float kProgressBarHeight = 5.0f;
	if (CurrentObject->PreviewImage != nullptr)
	{
		static const FVector2D kPreviewImageSize(180.0f, 180.0f);
		if (CurrentObject->PreviewBrush == nullptr)
		{
			CurrentObject->PreviewBrush = new FSlateImageBrush(CurrentObject->PreviewImage, kPreviewImageSize);
		}

		// show graph meta data with preview image
		DetailBuilder.HideCategory(TEXT("Information"));
		IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(CurrentObject->GraphFriendlyName));
		Category.SetSortOrder(SortOrder++);
		Category.AddCustomRow(FText::FromString(TEXT("New")))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.Padding(0.0f, 0.0f, 20.0f, 0.0f)
				.AutoWidth()
				[
					SNew(SScaleBox)
						.Stretch(EStretch::None)
						.HAlign(HAlign_Center)
						.OverrideScreenSize(kPreviewImageSize)
						[
							SNew(SImage)
								.Image(CurrentObject->PreviewBrush)
						]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					FInstaMATImporterUIUtilities::CreateMetaDataPanel(CurrentObject->GraphFriendlyName, CurrentObject->Category, CurrentObject->Documentation, CurrentObject->Author, CurrentObject->URL, CurrentObject->Version, CurrentObject->Tags)
				]
			]
			+SVerticalBox::Slot()
			.MaxHeight(kProgressBarHeight)
			.Padding(kPadding)
			[
				SNew(SProgressBar)
				.Visibility(AsyncOperationInProcessVisibilityAttribute)
				.Percent(ExecutionProgressAttribute)
			]
		];
	}
	else
	{
		// show graph meta data
		DetailBuilder.HideCategory(TEXT("Information"));
		IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(CurrentObject->GraphFriendlyName));
		Category.SetSortOrder(SortOrder++);
		Category.AddCustomRow(FText::FromString(TEXT("New")))
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					FInstaMATImporterUIUtilities::CreateMetaDataPanel(CurrentObject->GraphFriendlyName, CurrentObject->Category, CurrentObject->Documentation, CurrentObject->Author, CurrentObject->URL, CurrentObject->Version, CurrentObject->Tags)
				]
				+ SVerticalBox::Slot()
				.MaxHeight(kProgressBarHeight)
				.Padding(kPadding)
				[
					SNew(SProgressBar)
						.Visibility(AsyncOperationInProcessVisibilityAttribute)
						.Percent(ExecutionProgressAttribute)
				]
			];
	}
	
	// only change order for resolution items
	IDetailCategoryBuilder& PreviewResolutionCategory = DetailBuilder.EditCategory(TEXT("InstanceMode"), FText::GetEmpty(), ECategoryPriority::Default);
	PreviewResolutionCategory.SetSortOrder(SortOrder++);

	TSharedRef<IPropertyHandle> UpdateTypePropertyHandle = DetailBuilder.GetProperty(FName(TEXT("UpdateType")));
	UpdateTypePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this]()
		{
			this->ForceRefreshUpdate();
		}));

	IDetailCategoryBuilder& ResolutionCategory = DetailBuilder.EditCategory(TEXT("ElementFormat"), FText::GetEmpty(), ECategoryPriority::Default);
	ResolutionCategory.SetCategoryVisibility(CurrentObject->OutputParameters.Num() > 0u);
	ResolutionCategory.SetSortOrder(SortOrder++);

	// Disable reset to default behavior
	TArray<TSharedRef<IPropertyHandle>> ResolutionProperties;
	ResolutionCategory.GetDefaultProperties(ResolutionProperties);

	for (TSharedRef<IPropertyHandle>& Property : ResolutionProperties)
	{
		Property.Get().MarkResetToDefaultCustomized(true);
		IDetailPropertyRow& Row = ResolutionCategory.AddProperty(Property);
		Row.OverrideResetToDefault(FResetToDefaultOverride::Create(/*bIsVisible:*/false, FSimpleDelegate()));
		Row.EditCondition(AsyncOperationInProcessAttribute, FOnBooleanValueChanged());
	}

	bool bIsInstancePropertiesVisible = CurrentObject->DefaultSeed != ~0u;

	// seed visualization
	TSharedRef<IPropertyHandle> Handle = DetailBuilder.GetProperty(TEXT("Seed"), UInstaMATImporterGraphInstance::StaticClass());
	if (bIsInstancePropertiesVisible)
	{
		IDetailCategoryBuilder& RandomizationCategory = DetailBuilder.EditCategory(TEXT("InstanceProperties"), FText::GetEmpty(), ECategoryPriority::Default);
		RandomizationCategory.InitiallyCollapsed(true);
		RandomizationCategory.SetSortOrder(SortOrder++);

		IDetailPropertyRow& Row = RandomizationCategory.AddProperty(Handle);

		const FIsResetToDefaultVisible fnIsResetButtonVisibleLambda = FIsResetToDefaultVisible::CreateLambda([WeakObject](TSharedPtr<IPropertyHandle> Property)
		{
			if (!WeakObject.IsValid())
				return false;

			return WeakObject->Seed != WeakObject->DefaultSeed;
		});
		const FResetToDefaultHandler fnResetToDefaultLambda = FResetToDefaultHandler::CreateLambda([WeakObject](TSharedPtr<IPropertyHandle> Property)
		{
			if (!WeakObject.IsValid())
				return;

			WeakObject->Seed = WeakObject->DefaultSeed;
			WeakObject->SetDirty(true);
		});
		const FResetToDefaultOverride ResetOverrideBehavior = FResetToDefaultOverride::Create(fnIsResetButtonVisibleLambda, fnResetToDefaultLambda, true);

		Row.EditCondition(AsyncOperationInProcessAttribute, FOnBooleanValueChanged());
		Row.OverrideResetToDefault(ResetOverrideBehavior);
	}
	else
	{
		DetailBuilder.HideProperty(Handle);
	}

	if (!CurrentObject->bIsGrayScalePermutable)
	{
		TSharedRef<IPropertyHandle> GrayScaleHandle = DetailBuilder.GetProperty(TEXT("bIsGrayScalePermutation"), UInstaMATImporterGraphInstance::StaticClass());
		DetailBuilder.HideProperty(GrayScaleHandle);
	}
	else
	{
		bIsInstancePropertiesVisible = true;
	}

	if (!bIsInstancePropertiesVisible)
	{
		DetailBuilder.HideCategory(TEXT("InstanceProperties"));
	}

	// input settings
	{
		DetailBuilder.HideCategory(TEXT("InputParameters"));

		/// The fnCreateInputGroupsForCategory lambda creates the inputs UI for a specified category
		const auto fnCreateInputGroupsForCategory = [/*copy:*/ CurrentObject](const FString& Category, IDetailCategoryBuilder& InputCategoryBuilder)
		{
			const TArray<UInstaMATInputBase*> Inputs = CurrentObject->GetInputsByCategory(Category);

			for (UInstaMATInputBase* const Input : Inputs)
			{
				// NOTE: this may happen after restarting the engine.
				// we don't want the parent to be an UProperty (yet)
				if (Input->Parent == nullptr)
				{
					Input->Parent = CurrentObject;
				}

				InstaMATUIHelper::CreateInputGroup(Input, InputCategoryBuilder, Category);
			}
		};

		// get input parameters without category
		{
			IDetailCategoryBuilder& InputCategoryBuilder = DetailBuilder.EditCategory(FName(TEXT("Input Settings")), FText::GetEmpty(), ECategoryPriority::Default);
			InputCategoryBuilder.SetSortOrder(SortOrder++);
			InputCategoryBuilder.InitiallyCollapsed(true);
			fnCreateInputGroupsForCategory(FString(), InputCategoryBuilder);
		}

		// create categories
		const TArray<FString> SortedCategories = CurrentObject->GetSortedInputCategories();

		for (const FString& Category : SortedCategories)
		{
			IDetailCategoryBuilder& InputCategoryBuilder = DetailBuilder.EditCategory(FName(Category), FText::GetEmpty(), ECategoryPriority::Default);
			InputCategoryBuilder.SetSortOrder(SortOrder++);
			InputCategoryBuilder.InitiallyCollapsed(true);
			fnCreateInputGroupsForCategory(Category, InputCategoryBuilder);
		}
	}

	// show outputs
	{
		/// The fnOutputImagesView lambda generates a table for all output images
		const auto& fnOutputImagesView = [](const TArray<UInstaMATOutput*>& OutputImages) -> TSharedRef<SVerticalBox>
		{
			const float kPadding = 5.0f;
			TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);

			const FText LinearText = FText::FromString(TEXT("Linear"));
			const FText SRGBText = FText::FromString(TEXT("sRGB"));

			for (UInstaMATOutput* const OutputImage : OutputImages)
			{
				if (OutputImage == nullptr || OutputImage->Output == nullptr)
					continue;

				if (!OutputImage->Brush.IsValid())
				{
					const FVector2D UIImageSize(kBrushSize, kBrushSize);
					OutputImage->Output->AddToRoot();
					OutputImage->Brush = MakeShared<FSlateDynamicImageBrush>(OutputImage->Output, UIImageSize, OutputImage->Output->GetFName());
				}

				VerticalBox->AddSlot()
				.Padding(kPadding)
				.AutoHeight()
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.AutoWidth()
					.Padding(kPadding, 0.0f)
					[
						SNew(SImage)
						.Image(OutputImage->Brush.Get())
					]
					+SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromName(OutputImage->Output->GetFName()))
						.Justification(ETextJustify::Left)
					]
					+SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(OutputImage->Output->SRGB ? SRGBText : LinearText)
						.Justification(ETextJustify::Left)
					]
				];
			}

			return VerticalBox;
		};

		if (CurrentObject->OutputParameters.Num() == 0 && CurrentObject->OutputMeshParameters.Num() == 0 && CurrentObject->OutputSceneParameters.Num() == 0)
			return;

		IDetailCategoryBuilder& OutputCategory = DetailBuilder.EditCategory(TEXT("Outputs"), FText::GetEmpty(), ECategoryPriority::Default);
		OutputCategory.SetSortOrder(SortOrder++);
		OutputCategory.InitiallyCollapsed(true);

		// No material for non-material graphs
		if (CurrentObject->bIsMaterialGraph && CurrentObject->MaterialInstance == nullptr)
		{
			/// The fnRecreateMaterialClick lambda recreates the material of this instance.
			const auto fnRecreateMaterialClick = [/*copy:*/ CurrentObject]()
			{
				check(CurrentObject != nullptr);
				check(CurrentObject->MaterialInstance == nullptr);

				TArray<UTexture2D*> OutputTextures;

				for (UInstaMATOutput* const Output : CurrentObject->OutputParameters)
				{
					if (Output == nullptr || Output->Output == nullptr)
						continue;

					OutputTextures.Add(Output->Output);
				}

				FString MaterialPackagePath;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 4
				MaterialPackagePath = FPaths::Combine(MaterialPackagePath, *FString::Printf(TEXT("M_%s_Material"), *CurrentObject->CustomName));
#else
				MaterialPackagePath = FPaths::Combine(MaterialPackagePath, *FString::Printf(TEXT("MI_%s_Material"), *CurrentObject->CustomName));
#endif

				UPackage* const MaterialPackage = CreatePackage(*MaterialPackagePath); 
				const FInstaMATMaterialParameters MaterialSettings(CurrentObject->DisplacementHeight, CurrentObject->PhysicalWidth, CurrentObject->PhysicalHeight, CurrentObject->bHasPhysicalSize, CurrentObject->bIsBaseColorGrayscale);
				CurrentObject->MaterialInstance = FInstaMATImporterUtility::CreateFlattenMaterialInstanceForTextures(MaterialPackage, OutputTextures, CurrentObject->CustomName + "_Material", MaterialSettings);
				return FReply::Handled(); 
			};

			FDetailWidgetRow& RecreateRow = OutputCategory.AddCustomRow(FText::FromString(TEXT("Recreate missing Material")));
			RecreateRow.WholeRowContent()
			[
				SNew(SInstaMATButton)
				.Text(TEXT("Recreate missing Material"))
				.OnClicked_Lambda(fnRecreateMaterialClick)
			];
		}

		if (CurrentObject->OutputParameters.Num() > 0)
		{
			bool bAreOutputImagesMissing = false;

			// check if output images are missing
			for (UInstaMATOutput* const OutputParameter : CurrentObject->OutputParameters)
			{
				if (OutputParameter->Output == nullptr)
				{
					bAreOutputImagesMissing = true;
					break;
				}
			}

			FDetailWidgetRow& Row = OutputCategory.AddCustomRow(FText::FromString(TEXT("Image Outputs")));
			Row.WholeRowContent()
			[
				fnOutputImagesView(CurrentObject->OutputParameters)
			];

			/// The fnSaveOutputsButtonClick lambda opens the SaveTextures dialog.
			const auto fnSaveOutputsButtonClick = [WeakObject]() -> FReply
			{
				ShowSaveTexturesDialog(WeakObject.Get());
				return FReply::Handled();
			};

			IDetailCategoryBuilder& SaveOutputsCategory = DetailBuilder.EditCategory(TEXT("Save Images To Disk"), FText::GetEmpty(), ECategoryPriority::Default);
			SaveOutputsCategory.InitiallyCollapsed(true);
			OutputCategory.SetSortOrder(SortOrder++);

			if (bAreOutputImagesMissing)
			{
				/// The fnReCreateMissingTexturesClick lambda recreates the missing UTextures of this object.
				const auto fnRecreateMissingTexturesClick = [WeakObject]() -> FReply
				{
					if (!WeakObject.IsValid())
						return FReply::Handled();

					// recreate textures 
					FInstaMATImporterUtility::RecreateMissingOutputTexturesForGraphInstance(WeakObject.Get());

					TArray<UTexture2D*> Textures;
					{
						// collect textures
						Textures.Reserve(WeakObject->OutputParameters.Num());

						for (UInstaMATOutput* const Output : WeakObject->OutputParameters)
						{
							if (Output == nullptr)
								continue;

							check(Output->Output != nullptr);
							Textures.Add(Output->Output);
						}
					}

					// reconnect material to textures
					if (WeakObject->MaterialInstance != nullptr)
					{
						const FInstaMATMaterialParameters MaterialSettings(WeakObject->DisplacementHeight, WeakObject->PhysicalWidth, WeakObject->PhysicalHeight, WeakObject->bHasPhysicalSize, WeakObject->bIsBaseColorGrayscale);
						FInstaMATImporterUtility::ConnectTexturesToMaterial(Textures, WeakObject->MaterialInstance, MaterialSettings);
					}

					// enforce graph instance update and notifiy modules of changes
					WeakObject->SetDirty(true);
					FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
					PropertyEditorModule.NotifyCustomizationModuleChanged();

					return FReply::Handled();
				};

				FDetailWidgetRow& RecreateRow = OutputCategory.AddCustomRow(FText::FromString(TEXT("Recreate missing UTextures")));
				RecreateRow.WholeRowContent()
				[
					SNew(SInstaMATButton)
					.Text(TEXT("Recreate missing UTextures"))
					.OnClicked_Lambda(fnRecreateMissingTexturesClick)
				];
			}

			FDetailWidgetRow& SaveTexturesRow = OutputCategory.AddCustomRow(FText::FromString(TEXT("Save Textures To Disk")));
			SaveTexturesRow.WholeRowContent()
			[
				SNew(SInstaMATButton)
				.Text(TEXT("Save UTextures to Disk"))
				.OnClicked_Lambda(fnSaveOutputsButtonClick)
			];
		}

		if (CurrentObject->OutputMeshParameters.Num() > 0)
		{
			/// The fnOutputMeshesView lambda generates a table for all output meshes
			const auto& fnOutputMeshesView = [](const TArray<UInstaMATMeshOutput*>& OutputMeshes) -> TSharedRef<SVerticalBox>
			{
				const float kPadding = 5.0f;
				TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);

				for (UInstaMATMeshOutput* const OutputMesh : OutputMeshes)
				{
					if (OutputMesh == nullptr || OutputMesh->Output == nullptr)
						continue;

					// OutputMesh->Brush->SetResourceObject(OutputMesh->Output);

					VerticalBox->AddSlot()
					.Padding(kPadding)
					.AutoHeight()
					.VAlign(VAlign_Top)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.AutoWidth()
						.Padding(kPadding, 0.0f)
						[
							SNew(SImage)
							.Image(OutputMesh->Brush.Get())
						]
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromName(OutputMesh->Output->GetFName()))
							.Justification(ETextJustify::Left)
						]
					];
				}

				return VerticalBox;
			};

			bool bAreOutputMeshesMissing = false;

			// check if output meshes are missing
			for (UInstaMATMeshOutput* const OutputParameter : CurrentObject->OutputMeshParameters)
			{
				if (OutputParameter->Output == nullptr)
				{
					bAreOutputMeshesMissing = true;
					break;
				}
			}
			FDetailWidgetRow& Row = OutputCategory.AddCustomRow(FText::FromString(TEXT("Mesh Outputs")));
			Row.WholeRowContent()
			[
				fnOutputMeshesView(CurrentObject->OutputMeshParameters)
			];

			if (bAreOutputMeshesMissing)
			{
				/// The fnRecreateMissingMeshesClick lambda recreates missing static meshes.
				const auto fnRecreateMissingMeshesClick = [/*copy:*/ CurrentObject]() -> FReply
				{
					UPackage* const UnrealPackage = CurrentObject->GetPackage();
					check(UnrealPackage != nullptr);

					// get sub package
					const FString SubPackageName = FString::Format(TEXT("{0}{1}{2}"), { UnrealPackage->GetName(), TEXT("_Assets/"), CurrentObject->CustomName});
					UPackage* const SubPackage = CreatePackage(*SubPackageName);

					if (SubPackage == nullptr)
					{
						UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not create package='%s'"), *SubPackageName);
						return FReply::Handled();
					}

					check(SubPackage != nullptr);

					SubPackage->FullyLoad();
					SubPackage->Modify();

					for (UInstaMATMeshOutput* const OutputParameter : CurrentObject->OutputMeshParameters)
					{
						if (OutputParameter->Output != nullptr)
							continue;

						// create new static mesh
						OutputParameter->Output = NewObject<UStaticMesh>(SubPackage, *OutputParameter->OutputName, RF_Public | RF_Standalone);
						OutputParameter->Output->AddToRoot();
						OutputParameter->Output->SetNumSourceModels(1);

						FStaticMeshSourceModel& SourceModel = OutputParameter->Output->GetSourceModel(0);
						SourceModel.CreateMeshDescription();

						if (CurrentObject->MaterialInstance != nullptr)
						{
							FStaticMaterial NewStaticMaterial(CurrentObject->MaterialInstance);
							OutputParameter->Output->GetStaticMaterials().Add(NewStaticMaterial);
						}
					}

					// enforce graph instance update and notifiy modules of changes
					CurrentObject->SetDirty(true);
					FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
					PropertyEditorModule.NotifyCustomizationModuleChanged();

					return FReply::Handled();
				};

				FDetailWidgetRow& RecreateRow = OutputCategory.AddCustomRow(FText::FromString(TEXT("Recreate missing UStaticMeshes in Unreal Engine")));
				RecreateRow.WholeRowContent()
				[
					SNew(SInstaMATButton)
					.Text(TEXT("Recreate missing UStaticMeshes in Unreal Engine"))
					.OnClicked_Lambda(fnRecreateMissingMeshesClick)
				];
			}
		}

		if (CurrentObject->OutputSceneParameters.Num() > 0)
		{
			/// The fnOutputSceneView lambda draws a button for each scene to spawn actors in the level.
			const auto fnOutputSceneView = [](const TArray<UInstaMATGraphSceneOutput*>& OutputScenes) -> TSharedRef<SVerticalBox>
			{
				TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);
				
				for (UInstaMATGraphSceneOutput* const OutputScene : OutputScenes)
				{
					if (OutputScene == nullptr)
						continue;

					VerticalBox->AddSlot()
					.Padding(5.0f)
					.AutoHeight()
					.VAlign(VAlign_Top)
					[
						SNew(SInstaMATButton)
						.Text(TEXT("Spawn Scene in World"))
						.OnClicked_Lambda([/*copy:*/ OutputScene]()
						{
							OutputScene->SpawnActorsInScene();
							return FReply::Handled();
						})
					];
				}

				return VerticalBox;
			};

			FDetailWidgetRow& Row = OutputCategory.AddCustomRow(FText::FromString(TEXT("Scene Outputs")));
			Row.WholeRowContent()
			[
				fnOutputSceneView(CurrentObject->OutputSceneParameters)
			];
		}
	}

	// manual update button
	{
		IDetailCategoryBuilder& ExecuteCategory = DetailBuilder.EditCategory(FName(ExecuteCategoryName));

		ExecuteCategory.AddCustomRow(FText::FromString(TEXT("Update Instance")))
			.ShouldAutoExpand(true)
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				.Padding(kPadding)
				[
					SNew(SInstaMATButton)
						.Text(TEXT("Update Instance"))
						.IsEnabled_Lambda([WeakObject]() -> bool
						{
							if (!WeakObject.IsValid())
								return false;

							return WeakObject->UpdateType == EInstaMATUpdateType::InstaMAT_Manual && !InstaMATUIHelper::IsAsyncOperationInProgress();
						})
						.OnClicked_Lambda([WeakObject]() -> FReply
						{
							if (!WeakObject.IsValid())
								return FReply::Handled();

							WeakObject->Update();
							return FReply::Handled();
						})
				]
			];

		ExecuteCategory.SetCategoryVisibility(CurrentObject->UpdateType == EInstaMATUpdateType::InstaMAT_Manual);
	}
}

void FInstaMATImporterGraphInstanceCustomization::PendingDelete()
{
	// no op
}

void FInstaMATImporterGraphInstanceCustomization::ShowSaveTexturesDialog(const UInstaMATImporterGraphInstance *const Instance)
{
	check(Instance != nullptr);

	const float kDefaultPadding = 5.0f;

	/**
	 * The FInstaMATSaveTextureDialogSettings struct contains 
	 * all information for the save output settings.
	 */
	struct FInstaMATSaveTextureDialogSettings
	{
		FInstaMATSaveTextureDialogSettings(const UInstaMATImporterGraphInstance* const TargetInstance) : 
		Settings(),
		SaveTexturesOutputState(),
		Instance(TargetInstance)
		{
			check(Instance != nullptr);
		};
		FInstaMATExportTextureSettings Settings;					/**< Texture Format settings. */
		TMap<const UInstaMATOutput*, bool> SaveTexturesOutputState;	/**< The Settings for each output, whether it should be saved. */
		const UInstaMATImporterGraphInstance* Instance;				/**< The instance. */
	};

	TSharedPtr<FInstaMATSaveTextureDialogSettings> SaveTextureSettings = MakeShareable(new FInstaMATSaveTextureDialogSettings(Instance));
	const UEnum* const TextureTypeEnum = StaticEnum<EInstaMATTextureFileType>();
	const UEnum* const TextureRotationEnum = StaticEnum<EInstaMATRotation>();
	const UEnum* const TextureResolutionWidthEnum = StaticEnum<EInstaMATTextureSize>();
	const UEnum* const TextureResolutionHeightEnum = StaticEnum<EInstaMATTextureSize>();
	const UEnum* const TextureExecutionFormatEnum = StaticEnum<EInstaMATExecutionFormat>();

	/// The fnCreateImageLabelCheckBox lambda creates an image label toggle button view.
	const auto fnCreateImageLabelCheckBox = [kDefaultPadding, /*copy:*/ SaveTextureSettings](UInstaMATOutput* const OutputImage, const bool bIsChecked) -> TSharedRef<SHorizontalBox>
	{
		const float kMinimumLabelWidth = 450.0f;
		
		if (!OutputImage->Brush.IsValid())
		{
			const FVector2D UIImageSize(kBrushSize, kBrushSize);
			OutputImage->Output->AddToRoot();
			OutputImage->Brush = MakeShared<FSlateDynamicImageBrush>(OutputImage->Output, UIImageSize, OutputImage->Output->GetFName());
		}

		return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(kDefaultPadding)
			[
				SNew(SImage)
				.Image(OutputImage->Brush.Get())
			]
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(kDefaultPadding)
			[
				SNew(STextBlock)
				.MinDesiredWidth(kMinimumLabelWidth)
				.Text(FText::FromName(OutputImage->Output->GetFName()))
				.Justification(ETextJustify::Left)
			]
			+SHorizontalBox::Slot()
			.Padding(kDefaultPadding)
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(bIsChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.CheckBoxContentUsesAutoWidth(true)
				.OnCheckStateChanged_Lambda([SaveTextureSettings, OutputImage](ECheckBoxState State)
					{
						const bool bIsEnabledForExport = State == ECheckBoxState::Checked ? true : false;
						if (!SaveTextureSettings->SaveTexturesOutputState.Contains(OutputImage))
						{
							SaveTextureSettings->SaveTexturesOutputState.Add(OutputImage, bIsEnabledForExport);
						}
						else
						{
							SaveTextureSettings->SaveTexturesOutputState[OutputImage] = bIsEnabledForExport;
						}
					}
				)
			];
	};

	/// The fnCreateImageList lambda creates a image label toggle button view list.
	const auto fnCreateImageList = [kDefaultPadding, /*copy:*/ SaveTextureSettings, &fnCreateImageLabelCheckBox](const UInstaMATImporterGraphInstance* const Instance) -> TSharedRef<SVerticalBox>
	{
		TSharedRef<SVerticalBox> VBox = SNew(SVerticalBox);

		for (UInstaMATOutput* const Output : Instance->OutputParameters)
		{
			if (Output == nullptr || Output->Output == nullptr)
				continue;

			if (!SaveTextureSettings->SaveTexturesOutputState.Contains(Output))
			{
				SaveTextureSettings->SaveTexturesOutputState.Add(Output, true);
			}

			VBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				fnCreateImageLabelCheckBox(Output, SaveTextureSettings->SaveTexturesOutputState[Output])
			];
		}

		return VBox;
	};

	/// The fnGetFileTypeValue lambda is a getter for the Current Type.
	const auto fnGetFileTypeValue = [/*copy:*/ SaveTextureSettings]() -> int32 { return (int32) SaveTextureSettings->Settings.FileType; };

	/// The fnGetFileRotationValue lambda is a getter for the Current Rotation.
	const auto fnGetFileRotationValue = [/*copy:*/ SaveTextureSettings]() -> int32 { return (int32) SaveTextureSettings->Settings.Rotation; };

	/// The fnGetFileResolutionWidthValue lambda is a getter for the Current Wdith.
	const auto fnGetFileResolutionWidthValue = [/*copy:*/ SaveTextureSettings]() -> int32 { return (int32) SaveTextureSettings->Settings.Width; };

	/// The fnGetFileResolutionHeightValue lambda is a getter for the Current Height.
	const auto fnGetFileResolutionHeightValue = [/*copy:*/ SaveTextureSettings]() -> int32 { return (int32) SaveTextureSettings->Settings.Height; };

	/// The fnGetExecutionFormatValue lambda is a getter for the Current Execution Format.
	const auto fnGetExecutionFormatValue = [/*copy:*/ SaveTextureSettings]() -> int32 { return (int32) SaveTextureSettings->Settings.ExecutionFormat; };

	SCustomDialog::FButton CancelButton(NSLOCTEXT(LOCTEXT_NAMESPACE, "Cancel", "Cancel"));
	SCustomDialog::FButton OKButton(NSLOCTEXT(LOCTEXT_NAMESPACE, "OK", "OK"));

	OKButton.OnClicked.BindLambda([/*copy:*/ SaveTextureSettings]()
	{
		const FString DefaultSavePath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_EXPORT);

		// Could not open folder.
		FString OutFolder;
		if (!PromptUserForDirectory(OutFolder, NSLOCTEXT(LOCTEXT_NAMESPACE, "SaveDirectory", "Save Directory").ToString(), DefaultSavePath))
			return;

		FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, OutFolder);

		SaveTextureSettings->Instance->SaveOutputImagesToDisk(SaveTextureSettings->SaveTexturesOutputState, OutFolder, SaveTextureSettings->Settings);
		FPlatformProcess::ExploreFolder(*OutFolder);
	});

	SWindow::FArguments WindowArguments;
	WindowArguments.FocusWhenFirstShown(true);
	WindowArguments.AutoCenter(EAutoCenter::PreferredWorkArea);
	const float kSettingsPadding = 1.5f;

	TSharedRef<SCustomDialog> Dialog = SNew(SCustomDialog)
		.Title(NSLOCTEXT(LOCTEXT_NAMESPACE, "SaveImagesToDisk", "Save Images To Disk"))
		.AutoCloseOnButtonPress(true)
		.UseScrollBox(false)
		.WindowArguments(WindowArguments)
		.Content()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(kDefaultPadding)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(FText::FromString(Instance->CustomName))
				.TextStyle(FInstaMATPluginStyle::Get(), TEXT("InstaMAT.MetaData.Text.Name"))
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(kDefaultPadding)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(kDefaultPadding)
				[
					fnCreateImageList(Instance)
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(kSettingsPadding)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(TEXT("File Type")))
				]
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[ 
					SNew(SEnumComboBox, TextureTypeEnum)
					
					.OnEnumSelectionChanged_Lambda([/*copy:*/ SaveTextureSettings](int32 SelectionIndex, ESelectInfo::Type Type)
						{
							SaveTextureSettings->Settings.FileType = static_cast<EInstaMATTextureFileType>(SelectionIndex);
						})
					.CurrentValue_Lambda(fnGetFileTypeValue)
				]
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(kSettingsPadding)
			[
				SNew(SHorizontalBox)
				.Visibility_Lambda([/*copy:*/ SaveTextureSettings]() -> EVisibility {
					return SaveTextureSettings->Settings.FileType == EInstaMATTextureFileType::InstaMAT_JPG ? EVisibility::Visible : EVisibility::Collapsed;
				})
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(TEXT("Quality")))
				]
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(SSlider)
						.MinValue(1.0f)
						.MaxValue(100.0f)
						.StepSize(1.0f)
						.Value_Lambda([/*copy:*/ SaveTextureSettings]() -> float
						{
							return FMath::Clamp(SaveTextureSettings->Settings.Quality, 1u, 100u);
						})
						.OnValueChanged_Lambda([/*copy:*/ SaveTextureSettings](const float Value)
						{
							SaveTextureSettings->Settings.Quality = FMath::RoundToInt(Value);
						})
					]

					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(SNumericEntryBox<uint32>)
						.MinValue(1u)
						.MaxValue(100u)
						.Value_Lambda([/*copy:*/ SaveTextureSettings]() -> uint32
						{
							return FMath::Clamp(SaveTextureSettings->Settings.Quality, 1u, 100u);
						})
						.OnValueChanged_Lambda([/*copy:*/ SaveTextureSettings](const uint32 Value)
						{
							SaveTextureSettings->Settings.Quality = FMath::Clamp(SaveTextureSettings->Settings.Quality, 1u, 100u);
						})
					]
				]
			]

			// FIXME: Custom rotations are disabled for now as anything besides 0 crashes the engine.
			
			//+SVerticalBox::Slot()
			//.AutoHeight()
			//.HAlign(HAlign_Fill)
			//.Padding(kDefaultPadding)
			//[
			//	SNew(SHorizontalBox)
			//	+SHorizontalBox::Slot()
			//	[
			//		SNew(STextBlock)
			//		.AutoWrapText(true)
			//		.Text(FText::FromString(TEXT("Rotation")))
			//	]
			//	+SHorizontalBox::Slot()
			//	[
			//		SNew(SEnumComboBox, TextureRotationEnum)
			//		.OnEnumSelectionChanged_Lambda([/*copy:*/ SaveTextureSettings](int32 SelectionIndex, ESelectInfo::Type Type)
			//			{
			//				SaveTextureSettings->Settings.Rotation = static_cast<EInstaMATRotation>(SelectionIndex);
			//			})
			//		.CurrentValue_Lambda(fnGetFileRotationValue)
			//	]
			//]

			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(kSettingsPadding)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(TEXT("Execution Format")))
				]
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(SEnumComboBox, TextureExecutionFormatEnum)
					.OnEnumSelectionChanged_Lambda([/*copy:*/ SaveTextureSettings](int32 SelectionIndex, ESelectInfo::Type Type)
						{
							SaveTextureSettings->Settings.ExecutionFormat = static_cast<EInstaMATExecutionFormat>(SelectionIndex);
						})
					.CurrentValue_Lambda(fnGetExecutionFormatValue)
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(kSettingsPadding)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(TEXT("Width")))
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(SEnumComboBox, TextureResolutionWidthEnum)
					.OnEnumSelectionChanged_Lambda([/*copy:*/ SaveTextureSettings](int32 SelectionIndex, ESelectInfo::Type Type)
						{
							SaveTextureSettings->Settings.Width = static_cast<EInstaMATTextureSize>(SelectionIndex);
						})
					.CurrentValue_Lambda(fnGetFileResolutionWidthValue)
				]
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(kSettingsPadding)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(TEXT("Height")))
				]
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(SEnumComboBox, TextureResolutionHeightEnum)
					.OnEnumSelectionChanged_Lambda([/*copy:*/ SaveTextureSettings](int32 SelectionIndex, ESelectInfo::Type Type)
						{
							SaveTextureSettings->Settings.Height = static_cast<EInstaMATTextureSize>(SelectionIndex);
						})
					.CurrentValue_Lambda(fnGetFileResolutionHeightValue)
				]
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(kSettingsPadding)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(FText::FromString(TEXT("Allow Dithering")))
				]
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
					.OnCheckStateChanged_Lambda([/*copy:*/ SaveTextureSettings](ECheckBoxState State)
					{
						SaveTextureSettings->Settings.bAllowDithering = State == ECheckBoxState::Checked;
					})
					.IsChecked_Lambda([/*copy:*/ SaveTextureSettings]()
					{
						return SaveTextureSettings->Settings.bAllowDithering ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				]
			]
		]
		.Buttons({ OKButton, CancelButton });
	
	Dialog->Show();
}

void FInstaMATImporterGraphInstanceCustomization::ForceRefreshUpdate()
{
	if (DetailLayoutBuilder == nullptr)
		return;

	DetailLayoutBuilder->ForceRefreshDetails();
}

#undef LOCTEXT_NAMESPACE
