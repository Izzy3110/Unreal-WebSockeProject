/**
 * InstaMATSettingsWindow.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSettingsWindow.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATSettingsWindow.h"
#include "InstaMATUIPCH.h"

#include "InstaMATModule.h"
#include "Slate/InstaMATPluginStyle.h"
#include "InstaMAT/InstaMATSettings.h"

#include "ISettingsModule.h"
#include "InstaMATGraphLibraryWindow.h"

#include "LevelEditor.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "IDocumentation.h"

#define LOCTEXT_NAMESPACE "InstaMAT"

SInstaMATSettingsWindow::SInstaMATSettingsWindow() : SCompoundWidget()
{
}

SInstaMATSettingsWindow::~SInstaMATSettingsWindow()
{
}

void SInstaMATSettingsWindow::Construct(const FArguments& InArgs)
{
	// we need the PropertyModule to create a DetailView
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowFavoriteSystem = true;
	DetailsViewArgs.bShowObjectLabel = false;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bShowScrollBar = false;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowOptions = true;

	DetailView = PropertyModule.CreateDetailView(DetailsViewArgs);
	UInstaMATSettings* const Settings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();

	FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	InstaMAT::IInstaMAT* const InstaMATAPI = InstaMATModule.GetInstaMATAPI();
	const FString SDKVersion = FString(UTF8_TO_TCHAR(InstaMATAPI->GetBuildDate()));

	const FSlateBrush* const LogoBrush = FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMAT.LogoTiny"));
	check(LogoBrush != nullptr);

	static const float kLogoPadding = 10.0f;

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.MaxHeight(LogoBrush->GetImageSize().Y + (kLogoPadding * 2.0f))
		.Padding(10.0f, 10.0f)
		[
			// InstaMAT Logo
			SNew(SImage)
			.Image(LogoBrush)
		]
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
			[ 
				// put a Scrollbox around the content area, in case someone resizes the window smaller
				SNew(SScrollBox)
				+SScrollBox::Slot()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Fill)
					[ 
						// detail view that shows the details of the used tools (UObject)
						DetailView->AsShared()
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.VAlign(VAlign_Bottom)
					.Padding(FMargin(10.0f, 25.0f, 10.0f, 25.0f))
					[
						SInstaMATSettingsWindow::CreateFooterTextBlock(FString::Format(TEXT("InstaMaterial GmbH 2018 - 2025\nhttp://www.InstaMaterial.com\n{0}"), { *SDKVersion }))
					]
				] 
			]
		]
	];

	UpdateUIContent();
}

void SInstaMATSettingsWindow::UpdateUIContent()
{
	UInstaMATSettings *const Settings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
	Settings->LoadConfig();
	DetailView->SetObject(Settings);

	Settings->UpdateDelegate = FSimpleDelegate::CreateSP(this, &SInstaMATSettingsWindow::ForceRefreshDetailsView);
}

void SInstaMATSettingsWindow::ForceRefreshDetailsView()
{
	DetailView->ForceRefresh();
}

#undef LOCTEXT_NAMESPACE
