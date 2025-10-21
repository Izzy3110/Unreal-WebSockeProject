/**
 * InstaMATImportGraphAsset.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImportGraphAsset.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATImporterGraphInstance.h"
#include "InstaMATImporterFactory.h"
#include "ParameterObjects/InstaMATInputBase.h"
#include "ParameterObjects/InstaMATGraphSceneOutput.h"
#include "InstaMAT/InstaMAT.h"
#include "InstaMAT/Public/InstaMATModule.h"
#include "InstaMAT/InstaMATSettings.h"
#include "InstaMATImporter/Private/InstaMATImageUtility.h"
#include "ParameterObjects/InstaMATOutput.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

static const float kInstaMATImporterGraphAssetMinimumDelayInSeconds = 0.015f;	/**< The minimum delay after a value change the reimport process is started. */
static const float kInstaMATImporterGraphAssetFastUpdateDelayInSeconds = 0.15f;	/**< The delay after a value change which only updates the GPU textures. */

UInstaMATImporterGraphInstance::UInstaMATImporterGraphInstance() : 
UObject(),
bIsDirty(false),
bIgnoreNextPropertyEvent(false),
bIsFormatSettingsDirty(true)
{
	FPlatformMisc::CreateGuid(this->UUID);
}

UInstaMATImporterGraphInstance::~UInstaMATImporterGraphInstance()
{
}

UInstaMATInputBase* UInstaMATImporterGraphInstance::GetInputParameterByName(const FString& InputParameterName)
{
	if (InputParameterName.IsEmpty())
		return nullptr;

	UInstaMATInputBase** const Input = InputParameters.FindByPredicate([&InputParameterName](UInstaMATInputBase* Input)
	{
		return Input->InputName == InputParameterName;
	});
	return *Input;
}

void UInstaMATImporterGraphInstance::SetDirty(bool bSetDirty, bool bNextPropertyEventIgnore)
{
	UWorld* const World = GEditor->GetEditorWorldContext().World();
	if (World == nullptr)
		return;

	bIsDirty = bSetDirty;
	bIgnoreNextPropertyEvent = bNextPropertyEventIgnore;
	LastDirtyStateChanged = World->RealTimeSeconds;

	FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	if (InstaMATModule.GetInstaMATInterface()->IsAsyncOperationInProgress())
		return;

	if (UpdateType == EInstaMATUpdateType::InstaMAT_Manual)
		return;

	UInstaMATSettings* const DefaultObject = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();

	// NOTE: we introduce a minimum delay as slider input would cause terrible stuttering
	const float UIDelay = DefaultObject->Delay < kInstaMATImporterGraphAssetMinimumDelayInSeconds ? kInstaMATImporterGraphAssetMinimumDelayInSeconds : DefaultObject->Delay;
	TSharedRef<FTimerManager> TimerManager = GEditor->GetTimerManager();
	if (TimerManager->IsTimerActive(FastUpdateTimerHandle))
	{
		TimerManager->ClearTimer(FastUpdateTimerHandle);
	}
	if (TimerManager->IsTimerActive(UpdateTimerHandle))
	{
		TimerManager->ClearTimer(UpdateTimerHandle);
	}

	// only call timer if setting dirty to true
	if (!bSetDirty)
		return;

	OutputParametersTextureDataSource.Empty();
	if (UIDelay >= kInstaMATImporterGraphAssetMinimumDelayInSeconds)
	{
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([UIDelay, this] {
				Update(/*bOnlyUpdateGPU*/ true);
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindLambda([UIDelay, this] {

					// Update CPU source
					for (UInstaMATOutput* const Parameter : OutputParameters)
					{
						if (!OutputParametersTextureDataSource.Contains(Parameter->OutputName))
							continue;

						const TSharedPtr<FInstaMATTextureDataSource> TextureDataSource = OutputParametersTextureDataSource[Parameter->OutputName];
						check(TextureDataSource.IsValid());

						UTexture2D* const DestinationTexture = Parameter->Output;
						InstaMATTextureUtility::FillUTexture2D(DestinationTexture, *TextureDataSource);
						DestinationTexture->ForceRebuildPlatformData();
						DestinationTexture->MarkPackageDirty();
						DestinationTexture->PostEditChange();
					}
					OutputParametersTextureDataSource.Empty();
				});
				TSharedRef<FTimerManager> TimerManager = GEditor->GetTimerManager();
				TimerManager->SetTimer(UpdateTimerHandle, TimerDelegate, UIDelay, /*InbLoop*/ true);
		});

		TimerManager->SetTimer(FastUpdateTimerHandle, TimerDelegate, kInstaMATImporterGraphAssetFastUpdateDelayInSeconds, /*InbLoop*/ false);
	}
	else
	{
		Update(/*bOnlyUpdateGPU*/ false);
	}
}

void UInstaMATImporterGraphInstance::BeginDestroy()
{
	Super::BeginDestroy();

	// clear any existing timers
	TSharedRef<FTimerManager> TimerManager = GEditor->GetTimerManager();
	if (TimerManager->IsTimerActive(FastUpdateTimerHandle))
	{
		TimerManager->ClearTimer(FastUpdateTimerHandle);
	}
	if (TimerManager->IsTimerActive(UpdateTimerHandle))
	{
		TimerManager->ClearTimer(UpdateTimerHandle);
	}
	OutputParametersTextureDataSource.Empty();
}

void UInstaMATImporterGraphInstance::SetOutputTextureSourceData(const FString& OutputName, const TSharedPtr<FInstaMATTextureDataSource>& TextureDataSource)
{
	OutputParametersTextureDataSource.Add(OutputName, TextureDataSource);
}

bool UInstaMATImporterGraphInstance::IsFormatSettingsDirty() const
{
	return bIsFormatSettingsDirty;
}

void UInstaMATImporterGraphInstance::SetFormatSettingsDirty(bool bIsSettingsDirty)
{
	bIsFormatSettingsDirty = bIsSettingsDirty;
}

void UInstaMATImporterGraphInstance::UpdateMaterialInstanceSettings(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (MaterialInstance == nullptr)
		return;

	/// The fnCreateScalarParameterValue lambda creates a scalar parameter info object.
	const auto fnCreateScalarParameterValue = [](const FName& Name, const float Value) -> FScalarParameterValue
	{
		FScalarParameterValue Parameter;
		Parameter.ParameterInfo = Name;
		Parameter.ParameterValue = Value;
		return Parameter;
	};

	const FName ScaleUParameterName(TEXT("Scale U"));
	const FName ScaleVParameterName(TEXT("Scale V"));
	FScalarParameterValue* ScalarParameter = MaterialInstance->ScalarParameterValues.FindByPredicate([ScaleUParameterName](const FScalarParameterValue& Value) { return Value.ParameterInfo.Name == ScaleUParameterName; });
	
	if (ScalarParameter == nullptr)
	{
		MaterialInstance->ScalarParameterValues.Add(fnCreateScalarParameterValue(ScaleUParameterName, ScaleU));
	}
	else
	{
		ScalarParameter->ParameterValue = ScaleU;
	}

	ScalarParameter = MaterialInstance->ScalarParameterValues.FindByPredicate([ScaleVParameterName](const FScalarParameterValue& Value) { return Value.ParameterInfo.Name == ScaleVParameterName; });

	if (ScalarParameter == nullptr)
	{
		MaterialInstance->ScalarParameterValues.Add(fnCreateScalarParameterValue(ScaleVParameterName, ScaleV));
	}
	else
	{
		ScalarParameter->ParameterValue = ScaleV;
	}

	if (bHasPhysicalSize)
	{
		FMaterialParameterInfo Info;
		Info.Association = EMaterialParameterAssociation::GlobalParameter;
		Info.Index = INDEX_NONE;
		Info.Name = FName(TEXT("UsePhysicalSize"));

		MaterialInstance->SetStaticSwitchParameterValueEditorOnly(Info, bEnablePhysicalSize);
	}

	{
		FMaterialParameterInfo Info;
		Info.Association = EMaterialParameterAssociation::GlobalParameter;
		Info.Index = INDEX_NONE;
		Info.Name = FName(TEXT("UseWorldAlignedTexture"));
	
		MaterialInstance->SetStaticSwitchParameterValueEditorOnly(Info, bEnableWorldAlignedTextures);
	}

	const FProperty* const PhysicalSizeProperty = UInstaMATImporterGraphInstance::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_STRING_CHECKED(UInstaMATImporterGraphInstance, bEnablePhysicalSize));
	check(PhysicalSizeProperty != nullptr);

	const FProperty* const WorldAlignedProperty = UInstaMATImporterGraphInstance::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_STRING_CHECKED(UInstaMATImporterGraphInstance, bEnableWorldAlignedTextures));
	check(WorldAlignedProperty != nullptr);

	const FName PropertyChangedName = PropertyChangedEvent.GetPropertyName();

	if (PropertyChangedName == PhysicalSizeProperty->GetName() ||
		PropertyChangedName == WorldAlignedProperty->GetName())
	{
		// Update the static permutation variables only if a static switch changed.
		MaterialInstance->UpdateStaticPermutation(/*FMaterialUpdateContext:*/ nullptr);
	}

	MaterialInstance->PostEditChange();
}

void UInstaMATImporterGraphInstance::Update(const bool bOnlyUpdateGPU)
{
	if (!bIgnoreNextPropertyEvent)
	{
		OutputParametersTextureDataSource.Empty();
		UInstaMATImporterFactory::UpdateGraphInstance(this, bOnlyUpdateGPU);
	}
	bIsDirty = false;
	bIgnoreNextPropertyEvent = false;
}

void UInstaMATImporterGraphInstance::SaveOutputImagesToDisk(const TMap<const class UInstaMATOutput*, bool>& SaveOutputs, const FString& Directory, const EInstaMATExecutionFormat Format, const EInstaMATRotation InRotation, const EInstaMATTextureFileType FileType, const EInstaMATTextureSize Width, const EInstaMATTextureSize Height) const
{
	FInstaMATExportTextureSettings Settings;
	Settings.ExecutionFormat = Format;
	Settings.Rotation = InRotation;
	Settings.Width = Width;
	Settings.Height = Height;

	SaveOutputImagesToDisk(SaveOutputs, Directory, Settings);
}

void UInstaMATImporterGraphInstance::SaveOutputImagesToDisk(const TMap<const class UInstaMATOutput*, bool>& SaveOutputs, const FString& Directory, const FInstaMATExportTextureSettings& ExportSettings) const
{
	UInstaMATImporterFactory::SaveOutputImagesToDiskForGraphInstance(this, SaveOutputs, Directory, ExportSettings);
}

TArray<UInstaMATInputBase*> UInstaMATImporterGraphInstance::GetInputsByCategory(const FString& InCategory)
{
	TArray<UInstaMATInputBase*> Inputs;
	if (InCategory.IsEmpty())
	{
		for (UInstaMATInputBase* const Input : InputParameters)
		{
			if (Input->Category.IsEmpty())
			{
				Inputs.Add(Input);
			}
		}
		return Inputs;
	}

	for (UInstaMATInputBase* const Input : InputParameters)
	{
		if (Input->Category.IsEmpty())
			continue;

		if (InCategory.Compare(Input->Category, ESearchCase::IgnoreCase) == 0)
		{
			Inputs.Add(Input);
		}
	}

	return Inputs;
}

bool UInstaMATImporterGraphInstance::EnsurePropertyChangeRequirementsAreApplied(FPropertyChangedEvent& PropertyChangedEvent)
{
	UInstaMATSettings* const DefaultObject = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();

	// Ensure texture locking is applied
	if (DefaultObject->bLockTextureResolution)
	{
		if (PropertyChangedEvent.GetPropertyName().IsEqual(TEXT("ResolutionWidth")))
		{
			ResolutionHeight = ResolutionWidth;
		}
		else if (PropertyChangedEvent.GetPropertyName().IsEqual(TEXT("ResolutionHeight")))
		{
			ResolutionWidth = ResolutionHeight;
		}
	}

	// Check if format changed.
	static const TArray<FName> kFormatProperties = { TEXT("Rotation"), TEXT("ResolutionWidth"), TEXT("ResolutionHeight"), TEXT("ExecutionFormat"), TEXT("bIsPreviewMode") };
	if (kFormatProperties.Contains(PropertyChangedEvent.GetPropertyName()))
	{
		bIsFormatSettingsDirty = true;
	}

	// Check if a material property that requires reallocation changed.
	static const TArray<FName> kRequiresReallocationProperties = { TEXT("bIsGrayScalePermutation") };
	if (kRequiresReallocationProperties.Contains(PropertyChangedEvent.GetPropertyName()))
	{
		bIsElementExecutionRequiredReallocation = true;
	}

	// Check if any material property changed.
	static const TArray<FName> kMaterialUpdateProperties = { TEXT("ScaleU"), TEXT("ScaleV"), TEXT("bEnablePhysicalSize"), TEXT("bEnableWorldAlignedTextures") };
	if (kMaterialUpdateProperties.Contains(PropertyChangedEvent.GetPropertyName()))
	{
		UpdateMaterialInstanceSettings(PropertyChangedEvent);
		return true;
	}

	return false;
}

TArray<FString> UInstaMATImporterGraphInstance::GetSortedInputCategories()
{
	TArray<FString> Categories;
	Categories.Reserve(8);

	for (UInstaMATInputBase* const Input : InputParameters)
	{
		if (Input->Category.IsEmpty())
			continue;

		Categories.AddUnique(Input->Category);
	}

	// Remove doubles
	Categories.RemoveAll([this](const FString& Item) { return InputCategoriesSorted.Contains(Item); });

	// Sort categories that are retrieve from the input parameters and are not part of the InputCategoriesSorted metadata.
	Categories.Sort();

	// Insert the categories from the meta data to the front
	Categories.Insert(InputCategoriesSorted, 0);

	return Categories;
}

bool UInstaMATImporterGraphInstance::IsElementExecutionReallocationRequired() const
{
	// NOTE: a restart is required if we disconnect a texture.
	for (UInstaMATInputBase* const Input : InputParameters)
	{
		if (Input->IsA<UInstaMATInputElementImage>())
		{
			if (Cast<UInstaMATInputElementImage>(Input)->bGraphInputRequiresReset)
				return true;
		}
	}

	return bIsElementExecutionRequiredReallocation;
}

void UInstaMATImporterGraphInstance::SetElementExecutionReallocationRequired(bool bIsRequired)
{
	bIsElementExecutionRequiredReallocation = bIsRequired;
}
