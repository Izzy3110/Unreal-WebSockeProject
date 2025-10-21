/**
 * InstaMATImporter.Build.cs (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporter.Build.cs
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

using UnrealBuildTool;
using System;
using System.Reflection;
using System.Globalization;

public class InstaMATImporter : ModuleRules
{
	public InstaMATImporter(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivatePCHHeaderFile = "Private/InstaMATImporterPCH.h";

		PrivateIncludePaths.AddRange(new string[] {
			"InstaMAT/Private",
			"InstaMAT/Public",
			"InstaMATImporter/Private"
		});

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"LevelEditor",
			"JsonUtilities",
			"InstaMAT",
			"InputCore",
			"RenderCore",
			"RHI",
			"AssetRegistry",
			"AssetTools",
			"MaterialUtilities",
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"ContentBrowser",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"EditorStyle",
			"RawMesh",
			"MeshDescriptionOperations",
			"MeshDescription",
			"DesktopPlatform",
			"StaticMeshDescription"
		});

		PrivateIncludePathModuleNames.AddRange(new string[] {
			"InstaMAT",
			"DesktopPlatform",
			"MaterialUtilities",
		});
	}
}
