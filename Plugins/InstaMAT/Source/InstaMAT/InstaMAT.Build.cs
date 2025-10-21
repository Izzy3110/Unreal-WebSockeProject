/**
* InstaMAT.Build.cs (InstaMAT)
*
* Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
* This file and all it's contents are proprietary and confidential.
*
* @file InstaMAT.Build.cs
* @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
* @section License
*/

using UnrealBuildTool;
using System;
using System.Reflection;
using System.Globalization;

public class InstaMAT : ModuleRules
{
	public InstaMAT(ReadOnlyTargetRules Target) : base(Target)
	{
		IWYUSupport = IWYUSupport.None;
		PrivatePCHHeaderFile = "InstaMATPCH.h";
		PublicDefinitions.Add("InstaMAT_LIB_DYNAMIC=1");

		PrivateIncludePaths.AddRange(new string[] {
			"InstaMAT/Private",
		});

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"CoreUObject",
			"Engine",
			"RHI",
			"Slate",
			"SlateCore",
			"InputCore",
			"UnrealEd",
			"EditorStyle",
			"StaticMeshDescription",
			"MeshDescription",
			"MeshDescriptionOperations",
			"Projects",
			"AssetTools",
			"RawMesh",
			"MaterialUtilities",
			"ImageWrapper"
		});

		PrivateIncludePathModuleNames.AddRange(new string[] {
			"Settings",
			"AssetTools",
			"MeshUtilities",
			"StaticMeshDescription",
			"MeshDescription",
			"MeshDescriptionOperations",
			"EditorStyle",
			"MaterialUtilities",
			"ImageWrapper",
		});
	}
}
