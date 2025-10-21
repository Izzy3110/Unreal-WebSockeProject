/**
 * InstaMATSettingsCustomization.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSettingsCustomization.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATUI/Private/Customizations/InstaMATSettingsCustomization.h"
#include "InstaMAT/InstaMATSettings.h"
#include "InstaMATUI/Private/InstaMATUIPCH.h"
#include "Slate/InstaMATPluginStyle.h"
#include "Slate/InstaMATButton.h"

#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SSpinBox.h"

#include "InstaMATModule.h"
#include "Slate/InstaMATGraphLibraryWindow.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"


FInstaMATSettingsCustomization::FInstaMATSettingsCustomization() :
DetailLayoutBuilder(nullptr)
{
}

FInstaMATSettingsCustomization::~FInstaMATSettingsCustomization()
{
	DetailLayoutBuilder = nullptr;
}

void FInstaMATSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	static const float kDefaultPadding = 5.0f; /**< The default margin used for UI elements. */
	static const float kFixedButtonsWidth = 200.0f; /**< A fixed size for all centralized buttons in the details page. */

	DetailLayoutBuilder = &DetailBuilder;

	// retrieve the settings tool from the currently edited Tool (Object)
	UClass* SettingsClass = nullptr;
	UObject* Instance = nullptr;
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingEdited;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingEdited);

	FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	IInstaMAT* const InstaMAT = InstaMATModule.GetInstaMATInterface();
	InstaMAT::IInstaMAT* const InstaMATAPI = InstaMATModule.GetInstaMATAPI();

	FText LicenseInformation;
	bool bIsAuthorized = false;

	// fetch license information and version
	if (InstaMATAPI != nullptr)
	{
		LicenseInformation = FText::FromString(ANSI_TO_TCHAR(InstaMATAPI->GetAuthorizationInformation())); 
		bIsAuthorized = InstaMATAPI->IsHostAuthorized();
	}

	if (bIsAuthorized && !InstaMATModule.bIsInstaMATFloatingLicenseAvailable)
	{
		InstaMATModule.bIsInstaMATFloatingLicenseAvailable = true;
		InstaMATModule.ForceLicenseRefreshDelegate = nullptr;
	}

	if (InstaMAT == nullptr)
		return;

	if (ObjectsBeingEdited.Num() == 1)
	{
		Instance = ObjectsBeingEdited[0].Get();
		if (Instance)
		{
			Settings = Cast<UInstaMATSettings>(Instance);
			SettingsClass = Settings->GetClass();
		}
	}
	else
	{
		return;
	}

	const int64 kKiloByte = 1024;

	int SortOrder = 0;
	IDetailCategoryBuilder& ResolutionCategory = DetailBuilder.EditCategory(TEXT("Texture Settings"), FText::GetEmpty(), ECategoryPriority::Default);
	ResolutionCategory.SetSortOrder(SortOrder++);

	IDetailCategoryBuilder& PreviewResolutionCategory = DetailBuilder.EditCategory(TEXT("Preview Texture Settings"), FText::GetEmpty(), ECategoryPriority::Default);
	PreviewResolutionCategory.SetSortOrder(SortOrder++);

	IDetailCategoryBuilder& MemorySettingsCategory = DetailBuilder.EditCategory(TEXT("VRAM Settings"), FText::GetEmpty(), ECategoryPriority::Default);
	MemorySettingsCategory.SetSortOrder(SortOrder++);

	IDetailCategoryBuilder& ExecutionSettingsCategory = DetailBuilder.EditCategory(TEXT("Execution Settings"), FText::GetEmpty(), ECategoryPriority::Default);
	ExecutionSettingsCategory.SetSortOrder(SortOrder++);

	IDetailCategoryBuilder& UISettingsCategory = DetailBuilder.EditCategory(TEXT("UI Settings"), FText::GetEmpty(), ECategoryPriority::Default);
	UISettingsCategory.SetSortOrder(SortOrder++);

	// Memory budget
	{
		DetailBuilder.HideProperty(TEXT("bEnableCustomMemorySetting"));

		IDetailCategoryBuilder& Builder = DetailBuilder.EditCategory(FName(TEXT("VRAM Settings")));

		const int64 TotalMemory = InstaMATAPI->GetTotalAvailableVideoMemory() / kKiloByte / kKiloByte;
		const float kPadding = 5.0f;

		/// The fnCreateInfoLabel lambda creates a info label with value.
		const auto fnCreateInfoLabel = [](const FString& Label, TFunction<FText()> TextLambda) -> TSharedPtr<SHorizontalBox>
		{
			if (Label.IsEmpty())
				return nullptr;

			TSharedPtr<SHorizontalBox> Box = SNew(SHorizontalBox);

			Box->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
				.Justification(ETextJustify::Left)
				.Text(FText::FromString(Label))
			];

			Box->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SNew(SEditableTextBox)
				.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(TEXT("InstaMATUI.EditableText")))
				.IsReadOnly(true)
				.Text_Lambda(MoveTemp(TextLambda))
				.Justification(ETextJustify::Left)
			];

			return Box;
		};

		const bool bIsCustomVRAMSettingAvailable = Settings->IsCustomVRAMSettingAvailable();

		{
			static const FString MemoryBudgetToolTip = TEXT("Specifies the video memory budget of InstaMAT. Note, this is not a hard limit as certain graph executions cannot be constrained. If a render operation would exceed the memory budget, InstaMAT will free GPU resources of the current graph.This may result in slower rendering performance when performing changes, as freed resources are required to be recomputed.");
			const int64 MaximumCustomMemoryBudget = TotalMemory * 0.9f;

			TSharedRef<IPropertyHandle> MemoryBudgetProperty = DetailBuilder.GetProperty(FName(TEXT("VRAMBudget")));
			check(MemoryBudgetProperty->IsValidHandle());
			MemoryBudgetProperty->MarkHiddenByCustomization();

			TWeakObjectPtr<UInstaMATSettings> WeakSettings = Settings;

			// Available Memory
			Builder.AddCustomRow(NSLOCTEXT(LOCTEXT_NAMESPACE, "Memory_Settings", "VRAM Settings"))
				.NameContent()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
						.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
						.Justification(ETextJustify::Left)
						.Text(FText::FromString(TEXT("Video Memory")))
				]
				.ValueContent()
				[ 
					SNew(SEditableTextBox)
						.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(TEXT("InstaMATUI.EditableText")))
						.IsReadOnly(true)
						.Text_Lambda([InstaMATAPI]() -> FText
							{
								const int64 TotalMemory = InstaMATAPI->GetTotalAvailableVideoMemory() / kKiloByte / kKiloByte;
								return FText::FromString(FString::Format(TEXT("{0}"), { TotalMemory }));
							})
						.Justification(ETextJustify::Left)
				];

			// Used memory
			Builder.AddCustomRow(NSLOCTEXT(LOCTEXT_NAMESPACE, "Memory_Settings", "VRAM Settings"))
				.NameContent()
				.HAlign(HAlign_Fill)
				[

					SNew(STextBlock)
						.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
						.Justification(ETextJustify::Left)
						.Text(FText::FromString(TEXT("Used Video Memory")))
				]
				.ValueContent()
				[ 
					SNew(SEditableTextBox)
						.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(TEXT("InstaMATUI.EditableText")))
						.IsReadOnly(true)
						.Text_Lambda([InstaMATAPI]() -> FText
							{
								const int64 CurrentBudgetInBytes = InstaMATAPI->GetUsedTextureMemoryInBytes() / kKiloByte / kKiloByte;
								return FText::FromString(FString::Format(TEXT("{0}"), { CurrentBudgetInBytes }));
							})
						.Justification(ETextJustify::Left)
				];

			// Enable custom memory management
			Builder.AddCustomRow(NSLOCTEXT(LOCTEXT_NAMESPACE, "Memory_Settings", "VRAM Settings"))
			.NameContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				.Visibility(bIsCustomVRAMSettingAvailable ? EVisibility::Visible : EVisibility::Collapsed)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
					.Justification(ETextJustify::Left)
					.ToolTipText(FText::FromString(MemoryBudgetToolTip))
					.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "Video_Memory_Budget", "Video Memory Budget"))
				]
				+SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 10.0f, 0.0f)
				.HAlign(HAlign_Right)
				[
					SNew(SCheckBox)
					.IsChecked(Settings->bEnableCustomMemorySetting)
					.OnCheckStateChanged_Lambda([/*copy:*/ InstaMATAPI, /*copy:*/  WeakSettings, /*copy:*/ InstaMAT](ECheckBoxState State)
					{
						if (!WeakSettings.IsValid())
							return;

						UInstaMATSettings* const SettingsObject = WeakSettings.Get();

						if (SettingsObject == nullptr)
							return;

						if (State == ECheckBoxState::Checked)
						{
							SettingsObject->VRAMBudget = InstaMAT->GetDefaultVRAMBudget();
						}
						else
						{
							SettingsObject->VRAMBudget = -1;
						}
						InstaMATAPI->SetVideoMemoryBudget(SettingsObject->VRAMBudget);

						SettingsObject->bEnableCustomMemorySetting = State == ECheckBoxState::Checked;
						SettingsObject->SaveToDefaultObject();
					})
				] 
			]
			.ValueContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.VAlign(EVerticalAlignment::VAlign_Center)
				.AutoHeight()
				[
					SNew(SSpinBox<int64>)
					.MinValue(kKiloByte)
					.MaxValue(MaximumCustomMemoryBudget)
					.ToolTipText(FText::FromString(MemoryBudgetToolTip))
					.Visibility_Lambda([/*copy:*/ WeakSettings]() {

							if (!WeakSettings.IsValid())
								return EVisibility::Collapsed;

							UInstaMATSettings* const SettingsObject = WeakSettings.Get();
							return SettingsObject->bEnableCustomMemorySetting ? EVisibility::Visible : EVisibility::Hidden;
						})
					.Value_Lambda([/*copy:*/InstaMATAPI, /*copy:*/WeakSettings, /*copy:*/ InstaMAT]() -> int64
					{
						if (!WeakSettings.IsValid())
							return InstaMAT->GetDefaultVRAMBudget();

						UInstaMATSettings* const SettingsObject = WeakSettings.Get();
						int64 CurrentMemoryBudget = WeakSettings->VRAMBudget;

						// Ensure default value is set if enabled
						if (CurrentMemoryBudget == -1)
						{
							CurrentMemoryBudget = InstaMAT->GetDefaultVRAMBudget();
							SettingsObject->VRAMBudget = CurrentMemoryBudget;
							InstaMATAPI->SetVideoMemoryBudget(SettingsObject->VRAMBudget);
						}

						return CurrentMemoryBudget / kKiloByte / kKiloByte;
					})
					.OnValueChanged_Lambda([InstaMATAPI, WeakSettings](int64 Value)
						{
							if (!WeakSettings.IsValid())
								return;

							UInstaMATSettings* const SettingsObject = WeakSettings.Get();

							if (SettingsObject == nullptr)
								return;

							SettingsObject->VRAMBudget = Value * kKiloByte * kKiloByte;
							SettingsObject->SaveToDefaultObject();
							InstaMATAPI->SetVideoMemoryBudget(SettingsObject->VRAMBudget);
						})
				]
			];
		}
	}

	if (!Settings->IsInstaMATConfigured())
	{
		IDetailCategoryBuilder& Builder = DetailBuilder.EditCategory(FName(TEXT("InstaMATConfiguration")));
		
		Builder.AddCustomRow(FText::FromString(TEXT("")))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "BadSetup_Title", "InstaMAT is not setup properly!"))
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FSlateColor(FLinearColor(FColor::Red)))
				.TextStyle(&FInstaMATPluginStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("InstaMAT.H2")))
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FSlateColor(FLinearColor(FColor::Red)))
				.TextStyle(&FInstaMATPluginStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("InstaMAT.H5")))
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "BadSetup_Message",
					"\n\n1. Please setup the InstaMAT Studio installation direction."
				))
			]
		];
	}
	
	// Buttons
	{
		DetailBuilder.HideCategory(FName(TEXT("Utility")));

		// create a button for each UFunction
		for (TFieldIterator<UFunction> FuncIt(SettingsClass); FuncIt; ++FuncIt)
		{
			UFunction* const Function = *FuncIt;

			// filter De-/Authorize functions, as we've already process them
			if (Function->HasAnyFunctionFlags(FUNC_Exec) &&
				(Function->NumParms == 0) &&
				!Function->GetMetaData(FName(TEXT("Category"))).Equals(TEXT("Deauthorize")) &&
				!Function->GetMetaData(FName(TEXT("Category"))).Equals(TEXT("Authorize")) && 
				!Function->GetMetaData(FName(TEXT("Category"))).Equals(TEXT("FloatingLicense")) && 
				!Function->GetMetaData(FName(TEXT("Category"))).Equals(TEXT("Path Settings")))
			{
				const FString FunctionName = Function->GetMetaData(FName(TEXT("DisplayName")));
				const FString ButtonCaption = FunctionName;
				const FString DetailCategoryName = Function->GetMetaData(FName(TEXT("Category")));

				IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory(FName(*DetailCategoryName));

				// create widgets
				CategoryBuilder.AddCustomRow(FText::FromString(DetailCategoryName))
				.WholeRowContent()
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					.WidthOverride(kFixedButtonsWidth)
					.Padding(kDefaultPadding)
					[
						SNew(SInstaMATButton)
						.Text(ButtonCaption)
						.OnClicked(FOnClicked::CreateStatic(&FInstaMATSettingsCustomization::ExecuteTool, &DetailBuilder, Function))
					]
				];
			}
		}
	}

	// License Information
	{
		IDetailCategoryBuilder& LicenseInfoCategory = DetailBuilder.EditCategory(TEXT("LicenseInfo"));

		// hide by default, as we will create a custom widget
		TSharedPtr<IPropertyHandle> LicenseInformationPropertyHandle = DetailBuilder.GetProperty(FName(TEXT("LicenseInformation")), UInstaMATSettings::StaticClass());

		check(LicenseInformationPropertyHandle->IsValidHandle());
		LicenseInformationPropertyHandle->MarkHiddenByCustomization();

		// create the custom widget
		LicenseInfoCategory.AddCustomRow(FText::FromString(TEXT("License")))
			.ShouldAutoExpand(false)
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
					.Padding(kDefaultPadding)
					.HAlign(HAlign_Fill)
					[
						SNew(STextBlock)
							.AutoWrapText(true)
							.Text(LicenseInformation)
					]
			];
	}

	// De-/ Authorization
	{
		// Buttons
		TSharedPtr<SVerticalBox> Buttons = SNew(SVerticalBox);

		FText CategoryName;
		FText WarningText;

		if (bIsAuthorized)
		{
			// hide the entire Authorize category
			DetailBuilder.HideCategory(TEXT("Authorize"));

			CategoryName = NSLOCTEXT(LOCTEXT_NAMESPACE, "SettingsCategoryName", "Deauthorize");
			WarningText = NSLOCTEXT(LOCTEXT_NAMESPACE, "SettingsWarning", "Deauthorization takes 24 hours to complete.\nThis node will remain locked until the deauthorization is finished.");

			for (TFieldIterator<UFunction> FuncIt(SettingsClass); FuncIt; ++FuncIt)
			{
				UFunction* const Function = *FuncIt;

				if (Function->HasAnyFunctionFlags(FUNC_Exec) &&
					(Function->NumParms == 0) &&
					Function->GetMetaData(FName(TEXT("Category"))).Equals(TEXT("Deauthorize")))
				{
					const FString FunctionName = Function->GetMetaData(FName(TEXT("DisplayName")));
					const FString ButtonCaption = FunctionName;
					const FString DetailCategoryName = Function->GetMetaData(FName(TEXT("Category")));

					IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(*DetailCategoryName));

					Buttons->AddSlot()
						.Padding(kDefaultPadding)
						[
							SNew(SInstaMATButton)
								.Text(ButtonCaption)
								.OnClicked(FOnClicked::CreateStatic(&FInstaMATSettingsCustomization::ExecuteTool, &DetailBuilder, Function))
						];
				}
			}
		}
		else
		{
			// hide the entire Deauthorize category
			DetailBuilder.HideCategory(TEXT("Deauthorize"));
			CategoryName = NSLOCTEXT(LOCTEXT_NAMESPACE, "SettingsCategoryName", "Authorize");

			FString ButtonsCategoryName;

			// Check whether the user is not authorized at all, or if its just the floating license that is unavailable.
			if (!InstaMATModule.bIsInstaMATFloatingLicenseAvailable)
			{
				ButtonsCategoryName = TEXT("FloatingLicense");
				WarningText = NSLOCTEXT(LOCTEXT_NAMESPACE, "SettingsWarning",
					"You have authorized InstaMAT with a floating license, but the license could not be verified by the Abstract servers. \n"
					"To continue using InstaMAT for Unreal Engine, please make sure that you're connected to the internet and that your license is not in use on another computer.");

			}
			else
			{
				ButtonsCategoryName = TEXT("Authorize");
				WarningText = NSLOCTEXT(LOCTEXT_NAMESPACE, "SettingsWarning",
					"InstaMAT requires a valid license. Please enter your licensing information in the fields below.\n"
					"In order to acquire a license an active internet connection is required.\n"
					"InstaMAT periodically connects to the InstaMAT servers to validate and refresh the license.");
			}


			for (TFieldIterator<UFunction> FuncIt(SettingsClass); FuncIt; ++FuncIt)
			{
				UFunction* const Function = *FuncIt;

				if (Function->HasAnyFunctionFlags(FUNC_Exec) &&
					(Function->NumParms == 0) &&
					Function->GetMetaData(FName(TEXT("Category"))).Equals(ButtonsCategoryName))
				{
					const FString FunctionName = Function->GetMetaData(FName(TEXT("DisplayName")));
					const FString ButtonCaption = FunctionName;
					const FString DetailCategoryName = Function->GetMetaData(FName(TEXT("Category")));

					IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(*DetailCategoryName));

					Buttons->AddSlot()
						.Padding(kDefaultPadding)
						[
							SNew(SInstaMATButton)
								.Text(ButtonCaption)
								.OnClicked(FOnClicked::CreateStatic(&FInstaMATSettingsCustomization::ExecuteTool, &DetailBuilder, Function))
						];
				}
			}
		}

		// hide the AccountName by default so we can customize it
		TSharedPtr<IPropertyHandle> AccountNamePropertyHandle = DetailBuilder.GetProperty(FName(TEXT("AccountName")), UInstaMATSettings::StaticClass());
		check(AccountNamePropertyHandle->IsValidHandle());
		AccountNamePropertyHandle->MarkHiddenByCustomization();

		// hide the SerialPassword by default so we can customize it
		TSharedPtr<IPropertyHandle> SerialPasswordPropertyHandle = DetailBuilder.GetProperty(FName(TEXT("SerialPassword")), UInstaMATSettings::StaticClass());
		check(SerialPasswordPropertyHandle->IsValidHandle());
		SerialPasswordPropertyHandle->MarkHiddenByCustomization();

		IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(*CategoryName.ToString()));

		TSharedPtr<SVerticalBox> MainVerticalBox = SNew(SVerticalBox);

		// Add Custom Widget
		Category.AddCustomRow(CategoryName)
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		[
			MainVerticalBox.ToSharedRef()
		];

		MainVerticalBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(kDefaultPadding, kDefaultPadding, kDefaultPadding, 20.f))
			[
				SNew(SBorder)
				.HAlign(HAlign_Fill)
				.Padding(kDefaultPadding)
				[
					// Info text
					SNew(STextBlock)
						.Text(WarningText)
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FSlateColor(FLinearColor(FColor::Red)))
				]
			];

		if (InstaMATModule.bIsInstaMATFloatingLicenseAvailable)
		{
			// Only display the authorization username and password if floating lisence is available.
			// Add username and password fields.
			MainVerticalBox->AddSlot()
				.AutoHeight()
				.Padding(kDefaultPadding)
				[
					// AccountName TextBox
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
								.Text(FText::FromString(AccountNamePropertyHandle->GetMetaData(FName(TEXT("DisplayName")))))
						]
						+ SHorizontalBox::Slot()
						[
							SNew(SEditableTextBox)
								.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(TEXT("InstaMATUI.EditableText")))
								.OnTextChanged(this, &FInstaMATSettingsCustomization::OnAccountNameChanged)
						]
				];

			MainVerticalBox->AddSlot()
				.AutoHeight()
				.Padding(kDefaultPadding)
				[
					// Password TextBox
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
								.Text(FText::FromString(SerialPasswordPropertyHandle->GetMetaData(FName(TEXT("DisplayName")))))
						]
						+ SHorizontalBox::Slot()
						[
							SNew(SEditableTextBox)
								.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(TEXT("InstaMATUI.EditableText")))
								.IsPassword(true)
								.OnTextChanged(this, &FInstaMATSettingsCustomization::OnPasswordChanged)
						]
				];

		}
		
		// Buttons slot.
		MainVerticalBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SBox)
					.WidthOverride(kFixedButtonsWidth)
					[
						Buttons->AsShared()
					]
			];

		if (!bIsAuthorized && InstaMATModule.bIsInstaMATFloatingLicenseAvailable)
		{
			const FString MachineKey = InstaMAT->GetMachineKeyAsFString();
			// Add Custom Widget
			Category.AddCustomRow(CategoryName)
				.WholeRowContent()
				.HAlign(HAlign_Fill)
				[

					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(kDefaultPadding)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Machine Key")))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(kDefaultPadding)
						[
							SNew(SBox)
								.HAlign(HAlign_Fill)
								[
									SNew(SEditableTextBox)
										.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(TEXT("InstaMATUI.EditableText")))
										.IsReadOnly(true)
										.Text(FText::FromString(MachineKey))
								]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
								.WidthOverride(kFixedButtonsWidth)
								[
									SNew(SInstaMATButton)
										.Text(TEXT("Ingest License"))
										.OnClicked(this, &FInstaMATSettingsCustomization::OnIngestLicenseButtonClicked)
								]
						]
				];
		}
	}

	// Path settings
	{
		IDetailCategoryBuilder& PathSettings = DetailBuilder.EditCategory(TEXT("Path Settings"));

		// Environment Folder
		{
			// hide by default, as we will create a custom widget
			TSharedPtr<IPropertyHandle> EnvironmentFolderHandle = DetailBuilder.GetProperty(FName(TEXT("EnvironmentFolder")), UInstaMATSettings::StaticClass());
			check(EnvironmentFolderHandle->IsValidHandle());
			EnvironmentFolderHandle->MarkHiddenByCustomization();

			// create the custom widget
			PathSettings.AddCustomRow(FText::FromString(TEXT("Environment Path")))
			.NameContent()
			.HAlign(HAlign_Fill)
			[
				SNew(STextBlock)
					.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
					.Text(FText::FromString(TEXT("InstaMAT Installation Directory")))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SEditableTextBox)
						.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(TEXT("InstaMATUI.EditableText")))
						.IsReadOnly(true)
						.Text(FText::FromString(Settings->EnvironmentFolder))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Fill)
				.Padding(kDefaultPadding, 0.0f)
				[
					SNew(SInstaMATButton)
						.Text(TEXT("Select Environment Path"))
						.OnClicked(FOnClicked::CreateSP(this, &FInstaMATSettingsCustomization::OnEnvironmentPathSelectionButtonClicked))
				]
			];
		}

		// User Folders property.
		PathSettings.AddProperty(GET_MEMBER_NAME_CHECKED(UInstaMATSettings, UserFolders));

		// Reload external paths
		{
			const FText CategoryName = NSLOCTEXT(LOCTEXT_NAMESPACE, "SettingsCategoryName", "Path Settings");

			TSharedPtr<SVerticalBox> Buttons = SNew(SVerticalBox);
			TSharedPtr<SVerticalBox> MainVerticalBox = SNew(SVerticalBox);

			for (TFieldIterator<UFunction> FuncIt(SettingsClass); FuncIt; ++FuncIt)
			{
				UFunction* const Function = *FuncIt;

				if (Function->HasAnyFunctionFlags(FUNC_Exec) && 
					Function->NumParms == 1 &&
					Function->GetMetaData(FName(TEXT("Category"))).Equals(TEXT("Path Settings")))
				{
					const FString FunctionName = Function->GetMetaData(FName(TEXT("DisplayName")));
					const FString ButtonCaption = FunctionName;
					const FString DetailCategoryName = Function->GetMetaData(FName(TEXT("Category")));

					IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(*DetailCategoryName));

					Buttons->AddSlot()
						.Padding(kDefaultPadding)
						[
							SNew(SInstaMATButton)
								.Text(ButtonCaption)
								.OnClicked(FOnClicked::CreateStatic(&FInstaMATSettingsCustomization::ExecuteTool, &DetailBuilder, Function))
						];
				}
			}

			// Add Custom Widget
			PathSettings.AddCustomRow(CategoryName)
				.WholeRowContent()
				.HAlign(HAlign_Fill)
				[
					MainVerticalBox.ToSharedRef()
				];

			// Buttons slot.
			MainVerticalBox->AddSlot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(kFixedButtonsWidth)
					[
						Buttons->AsShared()
					]
				];
		}
	}

	// Help and website links
	{
		IDetailCategoryBuilder& Builder = DetailBuilder.EditCategory(FName(TEXT("Documentation & Help")));

		Builder.AddCustomRow(FText::FromString(TEXT("")))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Justification(ETextJustify::Left)
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "WebsiteLabel", "Open InstaMAT "))
			]
			+SHorizontalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.AutoWidth()
			[
				SNew(SHyperlink)
				.OnNavigate_Lambda([]() { FPlatformProcess::LaunchURL(TEXT("https://www.InstaMaterial.com"), /*Parms:*/ nullptr, /*Error:*/ nullptr); })
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "Website", "Website"))
				.ToolTipText(NSLOCTEXT(LOCTEXT_NAMESPACE, "WebsiteToolTip", "Open the InstaMAT Website in a browser."))
			]
		];

		Builder.AddCustomRow(FText::FromString(""))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Justification(ETextJustify::Left)
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "WebsiteLabel", "Open InstaMAT "))
			]
			+SHorizontalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.AutoWidth()
			[
				SNew(SHyperlink)
				.OnNavigate_Lambda([]() { FPlatformProcess::LaunchURL(TEXT("https://docs.InstaMAT.io"), /*Parms:*/ nullptr, /*Error:*/ nullptr); })
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "Documentation", "Documentation"))
				.ToolTipText(NSLOCTEXT(LOCTEXT_NAMESPACE, "DocumentationToolTip", "Open the InstaMAT Documentation in a browser."))
			]
		];

		Builder.AddCustomRow(FText::FromString(""))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Justification(ETextJustify::Left)
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "WebsiteLabel", "Open InstaMAT "))
			]
			+SHorizontalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.AutoWidth()
			[
				SNew(SHyperlink)
				.OnNavigate_Lambda([]() { FPlatformProcess::LaunchURL(TEXT("https://community.TheAbstract.co"), /*Parms:*/ nullptr, /*Error:*/ nullptr); })
				.TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("SmallText")))
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "Community", "Community"))
				.ToolTipText(NSLOCTEXT(LOCTEXT_NAMESPACE, "CommunityToolTip", "Open the InstaMAT Community in a browser."))
			]
		];
	}
}

FReply FInstaMATSettingsCustomization::OnEnvironmentPathSelectionButtonClicked()
{
	static const FText Title = NSLOCTEXT(LOCTEXT_NAMESPACE, "UserMessageTitle", "InstaMAT for Unreal Engine");
	static const FText InvalidPathMessageText = NSLOCTEXT(LOCTEXT_NAMESPACE, "InvalidPathMessageText", "InstaMAT for Unreal Engine could not load the environment.\nPlease ensure that the correct InstaMAT Studio path is selected.");
	IDesktopPlatform* const Desktop = FDesktopPlatformModule::Get();

	if (Desktop == nullptr)
		return FReply::Handled();

	FString Directory;
	
#if PLATFORM_WINDOWS
	if (!Desktop->OpenDirectoryDialog(nullptr, TEXT("Select InstaMAT Studio installation Path"), Settings->EnvironmentFolder, Directory))
		return FReply::Handled();
#endif
	
#if PLATFORM_MAC
	int32 FilterIndex;
	TArray<FString> Files;

	// NOTE: The file extension type "*.app" is not currently working on latest macOS version. Using a generic file type "*.*" in the meantime so the user can select the InstaMAT Studio application.
	if (!Desktop->OpenFileDialog(nullptr, "Select InstaMAT Studio installation Path", Settings->EnvironmentFolder, "", "Application|*.*", EFileDialogFlags::None, Files, FilterIndex))
		return FReply::Handled();

	if (Files.Num() == 0)
		return FReply::Handled();
	
	Directory = FPaths::Combine(Files[0], "Contents/Resources/Environment");
	
	if (!FPaths::DirectoryExists(Directory))
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("Invalid InstaMAT Studio Path. Please navigate to Applications and select InstaMAT Studio."));
		return FReply::Handled();
	}
#endif

	if (!FInstaMATModule::IsInstaMATExecutableInPath(Directory))
	{
		FMessageDialog::Open(EAppMsgType::Ok, InvalidPathMessageText, Title);
		return FReply::Handled();
	}

	FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	IInstaMAT* const InstaMAT = InstaMATModule.GetInstaMATInterface();

	if (InstaMAT == nullptr)
		return FReply::Handled();

	if (InstaMAT->LoadEnvironmentPackageFromPath(Directory, /*bIsSystemLibrary:*/ true))
	{
		InstaMAT->GetGraphObjectLibraryPreviews(/*bEnforceRecache:*/ true);

		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT Studio path loaded."));

		Settings->EnvironmentFolder = Directory;
		Settings->SaveConfig();
		OnForceRefreshDetails();
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, InvalidPathMessageText, Title);
	}

	return FReply::Handled();
}

FReply FInstaMATSettingsCustomization::OnIngestLicenseButtonClicked()
{
	FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	IInstaMAT* const InstaMAT = InstaMATModule.GetInstaMATInterface();

	if (InstaMAT == nullptr)
		return FReply::Unhandled();

	TArray<FString> Files;

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	
	if (DesktopPlatform == nullptr)
		return FReply::Handled();

	if (DesktopPlatform->OpenFileDialog(nullptr, FString(TEXT("Ingest License")), FString(), FString(), FString(), /*SingleFile*/ EFileDialogFlags::None, Files))
	{
		if (InstaMAT->IngestLicense(Files[0]))
		{
			UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT is authorized."));
		}
		else
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT could not be authorized."));
		}
	}

	return FReply::Handled();
}

void FInstaMATSettingsCustomization::OnForceRefreshDetails()
{
	if (DetailLayoutBuilder == nullptr)
		return;

	DetailLayoutBuilder->ForceRefreshDetails();
}

FReply FInstaMATSettingsCustomization::ExecuteTool(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute)
{
	// get the edited tool and call the passed UFunction of it
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingEdited;
	DetailBuilder->GetObjectsBeingCustomized(ObjectsBeingEdited);

	if (ObjectsBeingEdited.Num() == 1)
	{
		if (UObject* const Instance = ObjectsBeingEdited[0].Get())
		{
			Instance->CallFunctionByNameWithArguments(*MethodToExecute->GetName(), *GLog, nullptr, true);
		}
	}

	return FReply::Handled();
}

void FInstaMATSettingsCustomization::OnAccountNameChanged(const FText& NewText)
{
	if (Settings == nullptr)
		return;

	Settings->AccountName = NewText;
}

void FInstaMATSettingsCustomization::OnPasswordChanged(const FText& NewText)
{
	if (Settings == nullptr)
		return;

	Settings->SerialPassword = NewText;
}

#undef LOCTEXT_NAMESPACE
