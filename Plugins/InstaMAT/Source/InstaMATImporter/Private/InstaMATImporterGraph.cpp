/**
 * InstaMATImporterGraph.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterGraph.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATImporterGraph.h"
#include "InstaMATImporterGraphInstance.h"
#include "InstaMATImporterFactory.h"

UInstaMATImporterGraph::UInstaMATImporterGraph() :
bIsMaterialGraph(false),
PreviewImage(nullptr),
ThumbnailImageCache(nullptr),
PreviewBrush(nullptr)
{
}

bool UInstaMATImporterGraph::IsCustomNameValid(const FString& Path, const FString& CustomName)
{
	FString Value = CustomName;

	// Remove whitespace
	Value.TrimStartAndEndInline();
	Value.RemoveSpacesInline();
	
	if (Value.IsEmpty())
		return false;

	const FName Name(CustomName);

	FText Error;

	if (!Name.IsValidObjectName(Error))
		return false;
	
	const FString FullPath = FPaths::Combine(Path, CustomName);
	return FindPackage(nullptr, *FullPath) == nullptr;
}

UInstaMATImporterGraphInstance* UInstaMATImporterGraph::CreateNewInstance(const FString& TargetDirectoryPath /* = FString()*/)
{
	return UInstaMATImporterFactory::CreateGraphInstance(this, TargetDirectoryPath);
}
