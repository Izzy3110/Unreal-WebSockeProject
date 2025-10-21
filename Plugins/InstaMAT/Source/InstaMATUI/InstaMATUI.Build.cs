/**
 * InstaMATUI.Build.cs (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATUI.Build.cs
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

using UnrealBuildTool;
using System;
using System.Reflection;
using System.Globalization;

public class InstaMATUI : ModuleRules
{
	public InstaMATUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivatePCHHeaderFile = "Private/InstaMATUIPCH.h";

		PrivateIncludePaths.AddRange(new string[] {
			"InstaMATUI/Private",
			"InstaMAT/Private"
		});

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"LevelEditor",				// Need the LevelEditor to add Toolbar Extensions and such things
			"InstaMAT",					// Include the Base Module so we can use Styles and the API
			"InputCore",				// ListView requires FKey, which requires this
			"RenderCore",				// For Operations
			"RHI",						// For Operations
			"AssetRegistry",			// To save new Assets
			"AssetTools",				// To duplicate Assets
			"UnrealEd",					// Thumbnail renderer
			"PropertyEditor",
			"Settings",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"EditorStyle",
			"EditorWidgets",
			"AppFramework",				// Color picker
			"WorkspaceMenuStructure",	// For menu
			"ToolWidgets",				// SCustomDialog
			"ToolMenus",				// FToolUIActionChoice
			"InstaMATImporter",			// Include Importer Module so we can create the UI for the objects
			"DesktopPlatform",			// To select a platform path
		});

		PrivateIncludePathModuleNames.AddRange(new string[] {
			"InstaMAT",
			"InstaMATImporter",
			"PropertyEditor",
		});
	}
}
