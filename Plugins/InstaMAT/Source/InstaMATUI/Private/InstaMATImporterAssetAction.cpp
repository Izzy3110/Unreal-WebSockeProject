/**
 * InstaMATImporterAssetAction.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterAssetAction.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATImporterAssetAction.h"
#include "InstaMATImporter/Public/InstaMATImporterGraphInstance.h"
#include "InstaMATImporter/Public/InstaMATImporterGraph.h"
#include "InstaMATImporter/Public/ParameterObjects/InstaMATOutput.h"
#include "InstaMATImporter/Public/ParameterObjects/InstaMATMeshOutput.h"
#include "InstaMATImporter/Public/InstaMATImporterFactory.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Slate/InstaMATPluginStyle.h"
#include "ObjectEditorUtils.h"
#include "ToolMenuSection.h"
#include "ToolMenuDelegates.h"
#include "Dialog/SCustomDialog.h"
#include "Misc/MessageDialog.h"
#include "ObjectTools.h"
#include "InstaMATUIModule.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"

/**
 * The InstaMATAssetActionUtilities namespace contains functions for asset actions.
 */
namespace InstaMATAssetActionUtilities
{
	/**
	 * Deletes the Graph and all it's children.
	 *
	 * @param Graph The graph.
	 */
	static void DeleteGraphAndAssets(UInstaMATImporterGraph* const Graph)
	{
		if (Graph == nullptr)
			return;

		const FText DialogMessage = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_DeleteGraphAndChildrenDialog", "Are you sure you want to delete the graph and all child assets?");
		const FText DialogTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_DeleteGraphAndChildrenDialogTitle", "Delete All Assets");

		FMessageDialog Dialog;
		const EAppReturnType::Type Result = Dialog.Open(EAppMsgType::OkCancel, EAppReturnType::Cancel, DialogMessage, DialogTitle);

		if (Result == EAppReturnType::Cancel)
			return;

		TArray<TWeakObjectPtr<UInstaMATImporterGraphInstance>> Instances;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		TArray<FAssetData> AssetsData;
		AssetRegistryModule.Get().GetAssetsByClass(UInstaMATImporterGraphInstance::StaticClass()->GetClassPathName(), AssetsData);
		for (const FAssetData& AssetData : AssetsData)
		{
			UObject* const Asset = AssetData.GetAsset();
			if (Asset == nullptr || !Asset->IsA<UInstaMATImporterGraphInstance>())
				continue;

			UInstaMATImporterGraphInstance* const GraphInstanceAsset = Cast<UInstaMATImporterGraphInstance>(Asset);
			if (GraphInstanceAsset->GraphID != Graph->GraphID)
				continue;

			Instances.Add(GraphInstanceAsset);
		}

		TArray<UObject*> Garbage;
		for (TWeakObjectPtr<UInstaMATImporterGraphInstance> Instance : Instances)
		{
			if (!Instance.IsValid())
				continue;

			for (UInstaMATOutput* const OutputParameter : Instance->OutputParameters)
			{
				if (OutputParameter == nullptr || OutputParameter->Output == nullptr)
					continue;

				Garbage.Add(OutputParameter->Output);
			}
			Garbage.Append(Instance->OutputParameters);
			Instance->OutputParameters.Empty();

			for (UInstaMATMeshOutput* const Output : Instance->OutputMeshParameters)
			{
				if (Output == nullptr || Output->Output == nullptr)
					continue;

				Garbage.Add(Output->Output);
			}
			Garbage.Append(Instance->OutputMeshParameters);
			Instance->OutputMeshParameters.Empty();

			if (Instance->MaterialInstance != nullptr)
			{
				Garbage.Add(Instance->MaterialInstance);
			}
			Instance->MaterialInstance = nullptr;
		}

		for (TWeakObjectPtr<UInstaMATImporterGraphInstance> Instance : Instances)
		{
			Garbage.Add(Instance.Get());
		}

		Garbage.Add(Graph);
		ObjectTools::DeleteObjects(Garbage);
	}

	/**
	 * Delets the instance and it's assets.
	 *
	 * @param Instance the instance.
	 */
	static void DeleteInstanceAndAssets(UInstaMATImporterGraphInstance* const Instance)
	{
		if (Instance == nullptr)
			return;

		TArray<UObject*> Garbage;
		for (UInstaMATOutput* const OutputParameter : Instance->OutputParameters)
		{
			if (OutputParameter == nullptr || OutputParameter->Output == nullptr)
				continue;

			Garbage.Add(OutputParameter->Output);
		}
		Garbage.Append(Instance->OutputParameters);
		Instance->OutputParameters.SetNum(0);

		for (UInstaMATMeshOutput* const OutputParameter : Instance->OutputMeshParameters)
		{
			if (OutputParameter == nullptr || OutputParameter->Output == nullptr)
				continue;

			Garbage.Add(OutputParameter->Output);
		}
		Garbage.Append(Instance->OutputMeshParameters);
		Instance->OutputMeshParameters.SetNum(0);

		if (Instance->MaterialInstance != nullptr)
		{
			Garbage.Add(Instance->MaterialInstance);
		}
		Instance->MaterialInstance = nullptr;

		Garbage.Add(Instance);
		ObjectTools::DeleteObjects(Garbage); 
	}
}

FColor FInstaMATImporterGraphAssetAction::GetTypeColor() const
{
	return FColor::Blue;
}

UClass* FInstaMATImporterGraphAssetAction::GetSupportedClass() const
{
	return UInstaMATImporterGraph::StaticClass();
}

FText FInstaMATImporterGraphAssetAction::GetName() const
{
	return NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_Graph", "InstaMAT Graph");
}

bool FInstaMATImporterGraphAssetAction::HasActions(const TArray<UObject*>& InObjects) const
{
	TArray<TWeakObjectPtr<UInstaMATImporterGraph>> GraphAssets = GetTypedWeakObjectPtrs<UInstaMATImporterGraph>(InObjects);
	return GraphAssets.Num() == 1;
}

void FInstaMATImporterGraphAssetAction::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<UInstaMATImporterGraph>> GraphAssets = GetTypedWeakObjectPtrs<UInstaMATImporterGraph>(InObjects);

	if (GraphAssets.Num() != 1)
		return;

	UInstaMATImporterGraph* const Graph = GraphAssets[0].Get();
	check(Graph != nullptr);

	FExecuteAction CreateInstanceAction;
	CreateInstanceAction.BindStatic(&FInstaMATUIModule::CreateInstanceFromGraph, Graph);

	Section.AddMenuEntry(
		TEXT("InstaMAT_CreateInstance"),
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_CreateInstance", "Create Instance"),
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_CreateInstanceToolTip", "Creates an Instance of the material graph."),
		FSlateIcon(FInstaMATPluginStyle::GetStyleSetName(), "InstaMATUI.TabIcon"),
		FUIAction(CreateInstanceAction)
	);

	FExecuteAction DeleteAssetsAction;
	DeleteAssetsAction.BindStatic(&InstaMATAssetActionUtilities::DeleteGraphAndAssets, Graph);

	Section.AddMenuEntry(
		TEXT("InstaMAT_DeleteGraphAndChildren"),
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_DeleteGraphAndChildren", "Delete Graph and Instances"),
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_DeleteGraphAndChildrenToolTip", "Deletes the graph and all instances with their outputs."),
		FSlateIcon(FInstaMATPluginStyle::GetStyleSetName(), TEXT("InstaMATUI.TabIcon")),
		FUIAction(DeleteAssetsAction)
	);
}

TSharedPtr<class SWidget> FInstaMATImporterGraphAssetAction::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	const FSlateBrush* const Icon = FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.TabIcon"));

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetNoBrush())
		.Visibility(EVisibility::HitTestInvisible)
		.Padding(0.0f, 0.0f, 0.0f, 3.0f)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		[
			SNew(SImage)
			.Image(Icon)
		];
}

uint32 FInstaMATImporterGraphAssetAction::GetCategories()
{
	return EAssetTypeCategories::None;
}

TSharedPtr<class SWidget> FInstaMATImporterGraphInstanceAssetAction::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	static const FSlateBrush* const Icon = FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.TabIcon"));

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetNoBrush())
		.Visibility(EVisibility::HitTestInvisible)
		.Padding(0.0f, 0.0f, 0.0f, 3.0f)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		[
			SNew(SImage)
			.Image(Icon)
		];
}

FColor FInstaMATImporterGraphInstanceAssetAction::GetTypeColor() const
{
	return FColor(4, 169, 199);
}

UClass* FInstaMATImporterGraphInstanceAssetAction::GetSupportedClass() const
{
	return UInstaMATImporterGraphInstance::StaticClass();
}

FText FInstaMATImporterGraphInstanceAssetAction::GetName() const
{
	return NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_Graph_Instance", "InstaMAT Graph Instance");
}

uint32 FInstaMATImporterGraphInstanceAssetAction::GetCategories()
{
	return EAssetTypeCategories::None;
}

bool FInstaMATImporterGraphInstanceAssetAction::HasActions(const TArray<UObject*>& InObjects) const
{
	TArray<TWeakObjectPtr<UInstaMATImporterGraph>> GraphAssets = GetTypedWeakObjectPtrs<UInstaMATImporterGraph>(InObjects);
	return GraphAssets.Num() == 1;
}

void FInstaMATImporterGraphInstanceAssetAction::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<UInstaMATImporterGraphInstance>> GraphAssets = GetTypedWeakObjectPtrs<UInstaMATImporterGraphInstance>(InObjects);

	if (GraphAssets.Num() != 1)
		return;

	UInstaMATImporterGraphInstance* const Instance = GraphAssets[0].Get();
	check(Instance != nullptr);

	FExecuteAction DeleteAssetsAction;
	DeleteAssetsAction.BindStatic(&InstaMATAssetActionUtilities::DeleteInstanceAndAssets, Instance);

	Section.AddMenuEntry(TEXT("InstaMAT_DeleteInstanceAndChildren"),
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_DeleteInstanceAndChildren", "Delete Instance and Assets"),
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_DeleteInstanceAndChildrenToolTip", "Deletes the instance and all assets belonging to it."),
		FSlateIcon(FInstaMATPluginStyle::GetStyleSetName(), TEXT("InstaMATUI.TabIcon")),
		FUIAction(DeleteAssetsAction)
	);
}

#undef LOCTEXT_NAMESPACE