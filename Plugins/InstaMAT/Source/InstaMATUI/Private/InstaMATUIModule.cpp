/**
 * InstaMATUIModule.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATUIModule.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATUIModule.h"
#include "InstaMATUIPCH.h"

#include "InstaMATModule.h"
#include "InstaMATUICommands.h"
#include "Slate/InstaMATPluginStyle.h"

#include "Slate/InstaMATSettingsWindow.h"
#include "Slate/InstaMATGraphLibraryWindow.h"
#include "InstaMAT/InstaMATSettings.h"
#include "InstaMATImporterGraphInstanceThumbnailRenderer.h"
#include "InstaMATThumbnailRenderer.h"
#include "InstaMATImporterAssetAction.h"
#include "InstaMATImporter/Public/InstaMATImporterGraphInstance.h"
#include "InstaMATImporter/Public/InstaMATImporterGraph.h"
#include "Customizations/InstaMATSettingsCustomization.h"
#include "Customizations/InstaMATImporterGraphInstanceCustomization.h"
#include "Customizations/InstaMATImporterGraphCustomization.h"
#include "InstaMATImporterFactory.h"

#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "IAssetTools.h"
#include "Interfaces/IPluginManager.h"
#include "LevelEditor.h"
#include "PropertyEditorModule.h"
#include "EditorSupportDelegates.h"
#include "ISettingsModule.h"

#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

static const FName InstaMATSettingsWindowTabName(TEXT("InstaMATSettingsWindow"));
static const FName InstaMATLibraryWindowTabName(TEXT("InstaMATLibraryWindow"));

DEFINE_LOG_CATEGORY(LogInstaMAT);

#define LOCTEXT_NAMESPACE "InstaMATUI"

void FInstaMATUIModule::StartupModule()
{
	// register settings so we can retrieve them from everywhere
	ISettingsModule* const SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings"));
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("InstaMAT Settings"), FText::FromString(TEXT("InstaMAT Settings")), FText::FromString(TEXT("Configure InstaMAT for Unreal Engine")), GetMutableDefault<UInstaMATSettings>());
	}

	// register the UI commands class and create a new UICommandList
	FInstaMATUICommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);
	
	// bind the open window UI command to a function so we can utilize it
 	PluginCommands->MapAction(FInstaMATUICommands::Get().OpenInstaMATSettingsWindow,
 		FExecuteAction::CreateRaw(this, &FInstaMATUIModule::OpenInstaMATSettingsWindowClicked),
 		FCanExecuteAction());

	PluginCommands->MapAction(FInstaMATUICommands::Get().OpenInstaMATLibraryWindow,
		FExecuteAction::CreateRaw(this, &FInstaMATUIModule::OpenInstaMATLibraryWindowClicked),
		FCanExecuteAction());

	const FSlateIcon InstaMATIcon(FInstaMATPluginStyle::GetStyleSetName(), TEXT("InstaMATUI.TabIcon"));

	// register the InstaMAT tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(InstaMATSettingsWindowTabName);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(InstaMATSettingsWindowTabName, FOnSpawnTab::CreateRaw(this, &FInstaMATUIModule::OnSpawnInstaMATSettingsTab))
		.SetIcon(InstaMATIcon)
		.SetDisplayName(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATSettingsWindowTabTitle", "InstaMAT Settings"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(InstaMATLibraryWindowTabName);
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(InstaMATLibraryWindowTabName, FOnSpawnTab::CreateRaw(this, &FInstaMATUIModule::OnSpawnInstaMATLibraryTab))
		.SetIcon(InstaMATIcon)
		.SetDisplayName(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATLibraryWindowTabTitle", "InstaMAT Graph Library"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());

	UThumbnailManager::Get().RegisterCustomRenderer(UInstaMATImporterGraphInstance::StaticClass(), UInstaMATImporterGraphInstanceThumbnailRenderer::StaticClass());
	UThumbnailManager::Get().RegisterCustomRenderer(UInstaMATImporterGraph::StaticClass(), UInstaMATThumbnailRenderer::StaticClass());

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
	const TSharedRef<IAssetTypeActions>& GraphInstanceAction = MakeShareable(new FInstaMATImporterGraphInstanceAssetAction);
	AssetTools.RegisterAssetTypeActions(GraphInstanceAction);
	RegisteredAssetActions.Add(GraphInstanceAction);

	const TSharedRef<IAssetTypeActions>& GraphAction = MakeShareable(new FInstaMATImporterGraphAssetAction);
	AssetTools.RegisterAssetTypeActions(GraphAction);
	RegisteredAssetActions.Add(GraphAction);

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	// register details customizations
	PropertyModule.RegisterCustomClassLayout(UInstaMATSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FInstaMATUIModule::CreateSettingsCustomization));
	PropertyModule.RegisterCustomClassLayout(UInstaMATImporterGraphInstance::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FInstaMATUIModule::CreateGraphInstanceCustomization));
	PropertyModule.RegisterCustomClassLayout(UInstaMATImporterGraph::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateRaw(this, &FInstaMATUIModule::CreateGraphCustomization));

	PropertyModule.NotifyCustomizationModuleChanged();

	// register menu toolbar
	if (UToolMenu* const ToolBar = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.AssetsToolBar")))
	{
		FToolMenuSection& Content = ToolBar->FindOrAddSection(TEXT("InstaMAT"));
		
		/// The fnCreateMenu lambda generates the menu entries.
		const auto fnCreateMenu = [this](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.BeginSection("InstaMAT", NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATToolbarMenu", "InstaMAT Menu"));

			MenuBuilder.AddMenuEntry(
				TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([]()
					{
						return NSLOCTEXT(LOCTEXT_NAMESPACE, "GraphLibrary", "Library");
					})),
				TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([]()
					{
						return NSLOCTEXT(LOCTEXT_NAMESPACE, "GraphLibraryToolitp", "Opens the InstaMAT Graph Library.");
					})),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateRaw(this, &FInstaMATUIModule::OpenInstaMATLibraryWindowClicked),
							FCanExecuteAction()
						),
						NAME_None,
						EUserInterfaceActionType::Button
						);

			MenuBuilder.AddMenuEntry(
				TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([]()
					{
						return NSLOCTEXT(LOCTEXT_NAMESPACE, "Settings", "Settings");
					})),
				TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([]()
					{
						return NSLOCTEXT(LOCTEXT_NAMESPACE, "SettingsToolTip", "Opens the InstaMAT Settings window.");
					})),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateRaw(this, &FInstaMATUIModule::OpenInstaMATSettingsWindowClicked),
					FCanExecuteAction()
				),
				NAME_None,
				EUserInterfaceActionType::Button
				);

			MenuBuilder.EndSection();
		};

		FToolMenuEntry InstaMATButton = FToolMenuEntry::InitComboButton(
			TEXT("InstaMATMenu"),
			FUIAction(),
			FNewToolMenuChoice(FNewMenuDelegate::CreateLambda(fnCreateMenu)),
			NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT", "InstaMAT"),
			NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATToolTip", "InstaMAT Configuration"),
			FSlateIcon(FInstaMATPluginStyle::GetStyleSetName(), TEXT("InstaMATUI.TabIcon"))
		);
		InstaMATButton.SetCommandList(PluginCommands);

		Content.AddEntry(InstaMATButton);
	}

	InstallExtensions();
}

void FInstaMATUIModule::ShutdownModule()
{
	RemoveExtensions();

	if (ISettingsModule* const SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings")))
	{
		SettingsModule->UnregisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("InstaMAT Settings"));
	}

	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

		for (const TSharedRef<IAssetTypeActions> AssetAction : RegisteredAssetActions)
		{
			AssetTools.UnregisterAssetTypeActions(AssetAction);
		}
	}
	RegisteredAssetActions.Empty();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(InstaMATSettingsWindowTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(InstaMATLibraryWindowTabName);
}

void FInstaMATUIModule::OpenInstaMATSettingsWindowClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(InstaMATSettingsWindowTabName);
}

void FInstaMATUIModule::OpenInstaMATLibraryWindowClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(InstaMATLibraryWindowTabName);
}

void FInstaMATUIModule::CreateInstanceFromGraph(UInstaMATImporterGraph* const Graph)
{
	FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	static const FText ErrorMessageTitle = FText::FromString(TEXT("Failed to create new Instance."));

	if (Graph == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("InstaMAT graph is null.\n")), ErrorMessageTitle);
		return;
	}

	TWeakObjectPtr<UInstaMATImporterGraph> WeakGraph = Graph;

	FString NewName = FInstaMATImporterUtility::EnsureValidObjectName(Graph->GetName() + TEXT("_instance"));

	/// The fnCleanPrefix lambda removes the specified prefix from the value.
	const auto fnCleanPrefix = [](const FString& Prefix, const FString& Value) -> FString
	{
		if (Value.StartsWith(Prefix, ESearchCase::IgnoreCase))
			return Value.Right(Value.Len() - Prefix.Len());

		return Value;
	};

	// Common Unreal Engine Prefixes
	static const TArray<FString> kPrefixes = {
		TEXT("M_"),
		TEXT("MI_"),
		TEXT("T_"),
		TEXT("SK_"),
		TEXT("SM_"),
		TEXT("MATG_"),
		TEXT("MAT_"),
	};

	// Remove prefixes if part of the name
	for (const FString& Prefix : kPrefixes)
	{
		NewName = fnCleanPrefix(Prefix, NewName);
	}

	FSaveAssetDialogConfig SaveAssetDialogConfig;
	SaveAssetDialogConfig.DialogTitleOverride = FText(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_CreateInstance_Window", "Create new InstaMAT material instance"));
	SaveAssetDialogConfig.DefaultPath = InstaMATModule.GetDefaultPathForContentBrowser(ELastDirectory::NEW_ASSET, FPaths::GetPath(Graph->GetPathName(nullptr)));
	SaveAssetDialogConfig.AssetClassNames.Add(UInstaMATImporterGraphInstance::StaticClass()->GetClassPathName());
	SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;
	SaveAssetDialogConfig.DefaultAssetName = FString::Format(TEXT("MAT_{0}"), { NewName });

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	// NOTE: CreateModalSaveAssetDialog blocks this thread until the modal is closed.
	const FString& GraphPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);

	// Path is empty if the user canceled the operation. Simply return.
	if (GraphPath.IsEmpty())
		return;

	const FString InstanceNewPath = FPaths::GetPath(GraphPath);
	const FString NewInstanceFilename = FPaths::GetBaseFilename(GraphPath);

	if (!UInstaMATImporterGraph::IsCustomNameValid(InstanceNewPath, NewInstanceFilename))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Instance path is not valid. "
			"Please make sure that no other instance with the same name is in the same folder, and that the chosen path doesn't have any special characters.\n")), ErrorMessageTitle);
		return;
	}

	if (!WeakGraph.IsValid())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to create instance from graph. (Reason: The provided instance pointer is not valid.)\n")), ErrorMessageTitle);
		return;
	}

	Graph->NewInstanceName = NewInstanceFilename;
	if (Graph->CreateNewInstance(InstanceNewPath) == nullptr)
	{
		// FIXME: Ideally `CreateNewInstance()` would return an error message if failed.
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Could not create a new instance.\n")), ErrorMessageTitle);
	}

	// Clear the NewInstanceName regardless if the instance was created successfully or not.
	Graph->NewInstanceName = FString("");

	// Updates the last saved directory in Editor Utilities with the latest used.
	FEditorDirectories::Get().SetLastDirectory(ELastDirectory::NEW_ASSET, InstanceNewPath);
}

void FInstaMATUIModule::OnAssetEditorRequestedOpen(UObject* OpenedAsset)
{
}

TSharedRef<class SDockTab> FInstaMATUIModule::OnSpawnInstaMATSettingsTab(const class FSpawnTabArgs& SpawnTabArgs)
{
	// construct toolbar widget
	TSharedRef<SInstaMATSettingsWindow> InstaMATSettingsWindow = SNew(SInstaMATSettingsWindow);

	TSharedRef<SDockTab> DockTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			InstaMATSettingsWindow
		];

	DockTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FInstaMATUIModule::OnInstaMATTabClosed));

	return DockTab;
}

TSharedRef<class SDockTab> FInstaMATUIModule::OnSpawnInstaMATLibraryTab(const class FSpawnTabArgs& SpawnTabArgs)
{
	FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	UInstaMATSettings* const UserSettings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();

	// Construct toolbar widget
	TSharedRef<SInstaMATGraphLibraryWindow> InstaMATLibraryWindow =
		SNew(SInstaMATGraphLibraryWindow)
		.GraphObjects(InstaMATModule.GetInstaMATInterface()->GetGraphObjectLibraryPreviews(/*bEnforceRecache: */ UserSettings->bIsUserPathsChanged));

	TSharedRef<SDockTab> DockTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			InstaMATLibraryWindow
		];

	DockTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FInstaMATUIModule::OnInstaMATTabClosed));

	return DockTab;
}

void FInstaMATUIModule::OnModulesChanged(FName Module, EModuleChangeReason Reason)
{
	if (Module != TEXT("LevelEditor") || Reason != EModuleChangeReason::ModuleLoaded)
		return;

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	// add toolbar extension for settings
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender());
	ToolbarExtender->AddToolBarExtension(TEXT("Settings"), EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FInstaMATUIModule::AddToolbarExtension, FInstaMATUICommands::Get().OpenInstaMATSettingsWindow));
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);

	// add toolbar extension for library
	ToolbarExtender->AddToolBarExtension(TEXT("Library"), EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FInstaMATUIModule::AddToolbarExtension, FInstaMATUICommands::Get().OpenInstaMATLibraryWindow));
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);

	// add menu extension
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension(TEXT("General"), EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FInstaMATUIModule::AddMenuExtension, FInstaMATUICommands::Get().OpenInstaMATSettingsWindow));
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FInstaMATUIModule::InstallExtensions()
{
	FModuleManager::Get().OnModulesChanged().AddRaw(this, &FInstaMATUIModule::OnModulesChanged);

	if (FModuleManager::Get().IsModuleLoaded(TEXT("LevelEditor")))
	{
		OnModulesChanged(TEXT("LevelEditor"), EModuleChangeReason::ModuleLoaded);
	}
}

void FInstaMATUIModule::RemoveExtensions()
{
	FModuleManager::Get().OnModulesChanged().RemoveAll(this);
}

void FInstaMATUIModule::OnInstaMATTabClosed(TSharedRef<class SDockTab> ClosedTab)
{
}

TSharedRef<IDetailCustomization> FInstaMATUIModule::CreateSettingsCustomization()
{
	return MakeShareable(new FInstaMATSettingsCustomization);
}

TSharedRef<IDetailCustomization> FInstaMATUIModule::CreateGraphInstanceCustomization()
{
	return MakeShareable(new FInstaMATImporterGraphInstanceCustomization);
}

TSharedRef<IDetailCustomization> FInstaMATUIModule::CreateGraphCustomization()
{
	return MakeShareable(new FInstaMATImporterGraphCustomization);
}

void FInstaMATUIModule::AddToolbarExtension(FToolBarBuilder& Builder, TSharedPtr<FUICommandInfo> UICommand)
{
	Builder.AddToolBarButton(UICommand);
}

void FInstaMATUIModule::AddMenuExtension(FMenuBuilder& Builder, TSharedPtr<FUICommandInfo> UICommand)
{
	Builder.AddMenuEntry(UICommand);
}

IMPLEMENT_MODULE(FInstaMATUIModule, InstaMATUI);

#undef LOCTEXT_NAMESPACE
