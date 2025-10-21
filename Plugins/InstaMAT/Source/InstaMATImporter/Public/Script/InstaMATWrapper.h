/**
 * InstaMATWrapper.h (InstaMAT)
 *
 * Copyright 2019-2022 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATWrapper.h
 * @copyright 2019-2022 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMATModule.h"
#include "InstaMATImporterGraph.h"
#include "InstaMATImporterGraphInstance.h"
#include "InstaMATImporterFactory.h"
#include "InstaMATWrapper.generated.h"

/**
 * The UInstaMATWrapper provides an easy to use interface for Python and Blueprint.
 */
UCLASS(Blueprintable)
class UInstaMATWrapper : public UObject 
{
	GENERATED_BODY()

public:

	/**
	 * Imports an InstaMAT file from disk.
	 *
	 * @param FileName, the file name to import.
	 * @param TargetPath, the target path to import.
	 * @return The imported graph objects.
	 */
	UFUNCTION(BlueprintCallable)
	static TArray<UInstaMATImporterGraph*> ImportFileFromDisk(const FString& FileName, const FString& TargetPath)
	{
		if (FileName.IsEmpty() || TargetPath.IsEmpty())
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: File name and target path must be set."));
			return TArray<UInstaMATImporterGraph*>();
		}

		const FString AssetBaseName = FPackageName::GetShortName(FPaths::GetBaseFilename(FileName, /*removePath:*/ true));
		const FString AssetBasePath = FPackageName::GetLongPackagePath(TargetPath) + TEXT("/");
		FString PackageName = AssetBasePath + AssetBaseName;
		PackageName = PackageName.ConvertTabsToSpaces(1);
		PackageName.RemoveSpacesInline();

		UPackage* const Package = CreatePackage(*PackageName);
		
		if (Package == nullptr)
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not create package."));
			return TArray<UInstaMATImporterGraph*>();
		}
		
		Package->FullyLoad();

		return UInstaMATImporterFactory::ImportFile(Package, /*ignore:*/ FName(), FileName);
	}

	/**
	 * Creates an UInstaMATImporterGraphInstance from 
	 * the specified \p Graph object.
	 *
	 * @param InstanceName, the Instance name.
	 * @param TargetPath, the target path to save the instance in.
	 * @param Graph, the source graph.
	 * @return the newly created instance.
	 */
	UFUNCTION(BlueprintCallable)
	static UInstaMATImporterGraphInstance* CreateInstanceFromInstaMATObject(const FString& InstanceName, const FString& TargetPath, UInstaMATImporterGraph* const Graph)
	{
		// NOTE: TargetPath can be empty
		if (InstanceName.IsEmpty() || Graph == nullptr)
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: In order to create an Instance, provide a valid instance name and a UInstamATImporterGraph instance."));
			return nullptr;
		}

		Graph->NewInstanceName = InstanceName;
		UInstaMATImporterGraphInstance* const GraphInstance = UInstaMATImporterFactory::CreateGraphInstance(Graph, TargetPath);

		if (GraphInstance == nullptr)
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not create instance."));
		}

		return GraphInstance;
	}

	/**
	 * Saves the outputimages of the provided \p GraphInstance
	 * to the specified \p Path.
	 * 
	 * @param GraphInstance the graph instance.
	 * @param Path the save path.
	 * @param Format the execution format.
	 * @param Rotation the image rotation.
	 * @param FileType the image file type.
	 * @param Width the image width.
	 * @param Height the image height.
	 * @return True upon success.
	 */
	UFUNCTION(BlueprintCallable)
	static bool SaveOutputImagesToDisk(UInstaMATImporterGraphInstance* const GraphInstance, const FString& Path, const EInstaMATExecutionFormat Format, const EInstaMATRotation Rotation, const EInstaMATTextureFileType FileType, const EInstaMATTextureSize Width, const EInstaMATTextureSize Height)
	{
		if (GraphInstance == nullptr || Path.IsEmpty())
			return false;

		TMap<const UInstaMATOutput*, bool> OutputFiles;
		for (UInstaMATOutput* const Output : GraphInstance->OutputParameters)
		{
			OutputFiles[Output] = true;
		}

		GraphInstance->SaveOutputImagesToDisk(OutputFiles, Path, Format, Rotation, FileType, Width, Height);
		return true;
	}

	/**
	 * Imports the graphs with the specified \p Name 
	 * into the specified \p TargetPath.
	 * @note If an empty target path is specified,
	 * the import will open up a save dialog found matching graph.
	 *
	 * @param Name The Graph Name.
	 * @param TargetPath Optional target path.
	 * @return True upon success.
	 */
	UFUNCTION(BlueprintCallable)
	static bool ImportGraphFromLibraryWithName(const FString& Name, const FString& TargetPath)
	{
		if (Name.IsEmpty())
			return false;
		
		const FString AssetBasePath = FPackageName::GetLongPackagePath(TargetPath) + TEXT("/");
		FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
		IInstaMAT* const InstaMATInterface = InstaMATModule.GetInstaMATInterface();

		check(InstaMATInterface != nullptr);
		
		const TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> Graphs = InstaMATInterface->FindGraphObjectWithName(Name);

		if (Graphs.Num() == 0)
			return false;

		bool bImportedAtLeastOneGraph = false;
		for (const TSharedPtr<FInstaMATGraphObjectViewItem>& PreviewItem : Graphs)
		{
			if (UInstaMATImporterFactory::ImportGraphObjectWithID(PreviewItem->GraphID))
			{
				bImportedAtLeastOneGraph = true;
			}
		}

		return bImportedAtLeastOneGraph;
	}
};