/**
 * InstaMATImporterGraphCustomization.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraphCustomization.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATImporterGraphCustomization.h"
#include "InstaMATUIPCH.h"

#include "Slate/InstaMATPluginStyle.h"
#include "Slate/InstaMATButton.h"

#include "InstaMATImporter/Public/InstaMATImporterFactory.h"
#include "InstaMATImporter/Public/InstaMATImporterGraph.h"
#include "InstaMATImporter/Public/ParameterObjects/InstaMATInputBase.h"
#include "InstaMATImporterGraphInstanceCustomization.h"
#include "InstaMATImporterUIUtilities.h"
#include "InstaMAT/Public/InstaMATModule.h"
#include "InstaMATUIModule.h"

#include "Widgets/Layout/SScaleBox.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"

FInstaMATImporterGraphCustomization::FInstaMATImporterGraphCustomization() : DetailLayoutBuilder(nullptr)
{
}

FInstaMATImporterGraphCustomization::~FInstaMATImporterGraphCustomization()
{
	DetailLayoutBuilder = nullptr;
}

void FInstaMATImporterGraphCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailLayoutBuilder = &DetailBuilder;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingEdited;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingEdited);

	if (ObjectsBeingEdited.Num() != 1)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Can only edit single object."));
		return;
	}

	TWeakObjectPtr<UObject> CurrentUObject = ObjectsBeingEdited[0];

	if (CurrentUObject == nullptr)
		return;

	UInstaMATImporterGraph* const CurrentObject = Cast<UInstaMATImporterGraph>(CurrentUObject);

	if (CurrentObject == nullptr)
		return;

	// graph information
	{
		if (CurrentObject->PreviewImage == nullptr)
		{
			FInstaMATModule& Module = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));

			if (FInstaMATImporterUtility::IsCachedPreviewImageAvailable(CurrentObject->GraphID))
			{
				uint32 Width;
				uint32 Height;
				TArray<FColor> PixelData;

				if (Module.GetInstaMATInterface()->TryLoadingPreviewImageFromCache(CurrentObject->GraphID, CurrentObject->ImportFilePath, Width, Height, PixelData))
				{
					CurrentObject->PreviewImage = FInstaMATImporterUtility::CreateTextureFromBitmapData(Width, Height, PixelData);
				}
			}
			else
			{
				CurrentObject->PreviewImage = FInstaMATImporterUtility::CreatePreviewImage(Module, CurrentObject->GetPackage(), CurrentObject->ImportFilePath, FInstaMATImporterUtility::EnsureValidObjectName(FString(TEXT("Preview")) + CurrentObject->GetName()), CurrentObject->GraphID, /*bUseAlpha:*/ true);
			}
		}

		if (CurrentObject->PreviewImage != nullptr)
		{
			static const FVector2D kPreviewImageSize(180.0f, 180.0f);
			if (CurrentObject->PreviewBrush == nullptr)
			{
				CurrentObject->PreviewBrush = new FSlateImageBrush(CurrentObject->PreviewImage, kPreviewImageSize);
			}

			// show graph meta data with preview image
			DetailBuilder.HideCategory(TEXT("Information"));
			IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(TEXT("Graph Information")));
			Category.AddCustomRow(FText::FromString(TEXT("New")))
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
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
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					FInstaMATImporterUIUtilities::CreateMetaDataPanel(CurrentObject->GraphFriendlyName, CurrentObject->Category, CurrentObject->Documentation, CurrentObject->Author, CurrentObject->URL, CurrentObject->Version, CurrentObject->Tags)
				]
			];
		}
		else
		{
			// show graph meta data
			DetailBuilder.HideCategory(TEXT("Information"));
			IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(TEXT("Graph Information")));
			Category.AddCustomRow(FText::FromString(TEXT("New")))
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			[
				FInstaMATImporterUIUtilities::CreateMetaDataPanel(CurrentObject->GraphFriendlyName, CurrentObject->Category, CurrentObject->Documentation, CurrentObject->Author, CurrentObject->URL, CurrentObject->Version, CurrentObject->Tags)
			];
		}
	}

	// new instance button
	{
		TWeakObjectPtr<UInstaMATImporterGraph> WeakObject = CurrentObject;

		/// the fnTextChanged lambda applies the changed value to the captured variable
		const auto fnTextChanged = [WeakObject](const FText& NewText)
		{
			if (!WeakObject.IsValid())
				return;

			WeakObject->NewInstanceName = NewText.ToString();
		};
		
		const float kDefaultPadding = 5.0f;

		/// the fnNewInstanceButtonClicked lambda handles the button click and invokes the instance generation
		const auto fnNewInstanceButtonClicked = [WeakObject]() -> FReply
		{
			if (!WeakObject.IsValid())
				return FReply::Handled();

			UInstaMATImporterGraph* const SharedObject = WeakObject.Get();

			FInstaMATUIModule::CreateInstanceFromGraph(SharedObject);
			return FReply::Handled();
		};

		// show create new instance panel
		IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(FName(TEXT("Create New Instance")));
		Category.AddCustomRow(FText::FromString(TEXT("New")))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(kDefaultPadding)
			[
				SNew(SInstaMATButton)
				.Text(TEXT("Create New Instance"))
				.OnClicked(FOnClicked::CreateLambda(fnNewInstanceButtonClicked))
			]
		];
	}
}

void FInstaMATImporterGraphCustomization::PendingDelete()
{
}