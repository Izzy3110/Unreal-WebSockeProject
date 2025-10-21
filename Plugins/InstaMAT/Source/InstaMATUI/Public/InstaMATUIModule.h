/**
 * InstaMATUIModule.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATUIModule.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "IDetailCustomization.h"

class UInstaMATImporterGraph;

/**
 * The FInstaMATUIModule is the module handling all UI related tasks.
 */
class FInstaMATUIModule : public IModuleInterface
{
public:

	/**
	 * Called when the asset editor should be opened.
	 *
	 * @param OpenedAsset the asset.
	 */
	void OnAssetEditorRequestedOpen(UObject* OpenedAsset);

	/**
	 * Called on startup.
	 */
	virtual void StartupModule() override;

	/**
	 * Called on shutdown.
	 */
	virtual void ShutdownModule() override;

	/**
	 * Called from button click.
	 */
	void OpenInstaMATSettingsWindowClicked();

	/**
	 * Called from button click.
	 */
	void OpenInstaMATLibraryWindowClicked();

	/**
	 * Creates an Graph Instance for the specified Graph.
	 *
	 * @param Graph		The graph to create an Instance from.
	 */
	static void CreateInstanceFromGraph(UInstaMATImporterGraph* const Graph);
	
	TArray<TSharedRef<class IAssetTypeActions>> RegisteredAssetActions; /**< Registered asset actions*/

private:

	/** 
	 * Adds the Toolbar Button for opening the Window Tab.
	 *
	 * @param Builder Helper class to add Toolbar extensions.
	 */
	void AddToolbarExtension(FToolBarBuilder& Builder, TSharedPtr<FUICommandInfo> UICommand);

	/**
	 * Adds a Menu Button for opening the Winodw Tab.
	 *
	 * @param Builder Helper class to add Menu extensions.
	 */
	void AddMenuExtension(FMenuBuilder& Builder, TSharedPtr<FUICommandInfo> UICommand);

	/**
	 * Creates new DockTab and within it the Window Tab.
	 *
	 * @param SpawnTabArgs Holds Information about Index and Parent/Owner Window
	 * @return Reference to the Spawn SDockTab
	 */
	TSharedRef<class SDockTab> OnSpawnInstaMATSettingsTab(const class FSpawnTabArgs& SpawnTabArgs);

	/**
	 * Creates new DockTab and within it the Window Tab.
	 *
	 * @param SpawnTabArgs Holds Information about Index and Parent/Owner Window
	 * @return Reference to the Spawn SDockTab
	 */
	TSharedRef<class SDockTab> OnSpawnInstaMATLibraryTab(const class FSpawnTabArgs& SpawnTabArgs);

	/** 
	 * Called when the current Module Changes. 
	 *
	 * @param Module Module Name being loaded.
	 * @param Reason Reason why the Module got loaded.
	 */
	void OnModulesChanged(FName Module, EModuleChangeReason Reason);

	/**
	 * Installs the extensions. 
	 */
	void InstallExtensions();

	/**
	 * Removes the extensions.
	 */
	void RemoveExtensions();

	/**
	 * Called when the Tab is closed. Used for cleanup.
	 * 
	 * @param ClosedTab the closed tab.
	 */
	void OnInstaMATTabClosed(TSharedRef<class SDockTab> ClosedTab);

	/**
	 * Called to create an Instance of a DetailsCustomization.
	 *
	 * @return The customization.
	 */
	TSharedRef<IDetailCustomization> CreateSettingsCustomization();

	/**
	 * Called to create an Instance of a DetailsCustomization.
	 *
	 * @return The customization.
	 */
	TSharedRef<IDetailCustomization> CreateGraphInstanceCustomization();

	/**
	 * Called to create an Instance of a DetailsCustomization.
	 *
	 * @return The customization.
	 */
	TSharedRef<IDetailCustomization> CreateGraphCustomization();

	TSharedPtr<class FUICommandList> PluginCommands;	/**< UI Commands to bind to Slate Actions.*/
};
