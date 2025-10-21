/**
 * InstaMAT.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMAT.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMAT_h
#define InstaMAT_InstaMAT_h

#include "Runtime/Launch/Resources/Version.h"
#include "InstaMATEnum.h"
#include "MeshUtilities.h"
#include "InstaMATAPI.h"
#include "Slate/DeferredCleanupSlateBrush.h"

namespace InstaLOD
{
	class IInstaLOD;
	class IInstaLODMaterialData;
	class IInstaLODMaterial;
	class IInstaLODMesh;
	class IInstaLODSkeleton;
	class IInstaLODRenderMeshBase;
};

class InstaMATMesh;

static constexpr uint32 kInstaMATDownSampleResolutionThreshold = 512u; /**< Resolution below this threshold will be downsampled if not in preview mode. */

/**
 * The FInstaMATGraphObjectInputData struct contains information
 * about an input of the parent graph object.
 */
struct FInstaMATGraphObjectInputData
{
	FInstaMATGraphObjectInputData() :
	Name(),
	TypeString(),
	Documentation(),
	Category(),
	Type(InstaMAT::IGraphVariable::TypeInvalid),
	ControlType(InstaMAT::IGraphVariable::UIControlTypeSpinBox),
	StringDefaultValue()
	{
		FMemory::Memzero(&DefaultValue, sizeof(InstaMAT::ArithmeticGraphValue));
	}

	FString Name;											/**< The name of the input. */
	FString TypeString;										/**< The type of the input as a string. */
	FString Documentation;									/**< The documentation of the input. */
	FString Category;										/**< The category of the input. */
	InstaMAT::IGraphVariable::Type Type;					/**< The type of the input. */
	InstaMAT::IGraphVariable::UIControlType ControlType;	/**< The control type of the input. */
	InstaMAT::ArithmeticGraphValue DefaultValue;			/**< The default value. */
	FString StringDefaultValue;								/**< The string default value. */
};

/**
 * The FInstaMATGraphObjectOutputData struct contains information
 * about an output of the parent graph object.
 */
struct FInstaMATGraphObjectOutputData
{
	FInstaMATGraphObjectOutputData() :
	Name(),
	TypeString(),
	Documentation(),
	Type(InstaMAT::IGraphVariable::TypeInvalid),
	ColorSpace(InstaMAT::IGraphVariable::ColorSpaceTypeAuto)
	{
	}

	FString Name;											/**< The name of the output. */
	FString TypeString;										/**< The type of the output as a string. */
	FString Documentation;									/**< The documentation of the input. */
	InstaMAT::IGraphVariable::Type Type;					/**< The type of the output. */
	InstaMAT::IGraphVariable::ColorSpaceType ColorSpace;	/**< The color space of the output. */
};

/**
 * The FInstaMATGraphObjectViewItem is a representation of an InstaMAT Graph object.
 */
struct FInstaMATGraphObjectViewItem
{
	FInstaMATGraphObjectViewItem() :
	GraphID(),
	GraphFriendlyName(),
	GraphName(),
	Category(),
	Version(),
	Author(),
	Documentation(),
	URL(),
	Tags(),
	InputDefinitions(),
	Preview(nullptr),
	GraphPreviewBrush(),
	GraphSelectionBrush(),
	bIsUserGraph(false)
	{
	}

	FString GraphID;			/**< The graph id. */
	FString GraphFriendlyName;	/**< The graph friendly name. */
	FString GraphName;			/**< The graph full name. */
	FString Category;			/**< The graph category. */
	FString Version;			/**< The graph version. */
	FString Author;				/**< The graph author. */
	FString Documentation;		/**< The graph documentation. */
	FString URL;				/**< The graph url. */
	FString Tags;				/**< The graph tags. */

	TArray<FString> InputCategories;							/**< The input categories. */
	TArray<FInstaMATGraphObjectInputData> InputDefinitions;		/**< Holds information about all inputs of the graph. */
	TArray<FInstaMATGraphObjectOutputData> OutputDefinitions;	/**< Holds information baout all output of the graph. */

	class UTexture2D* Preview;									/**< The Preview Image. */
	TSharedPtr<FSlateDynamicImageBrush> GraphPreviewBrush;		/**< For painting in the graph preview. */
	TSharedPtr<FSlateDynamicImageBrush> GraphSelectionBrush;	/**< For painting in the graph selection. */
	bool bIsUserGraph;											/**< Whether the graph is a user graph. */
};

/**
 * The FInstaMATExportTextureSettings contains settings for texture export.
 */
struct FInstaMATExportTextureSettings
{
	FInstaMATExportTextureSettings() :
	ExecutionFormat (EInstaMATExecutionFormat::InstaMAT_Normalized16),
	Rotation(EInstaMATRotation::InstaMAT_Rotation0),
	Width(EInstaMATTextureSize::InstaMAT_1024),
	Height(EInstaMATTextureSize::InstaMAT_1024),
	FileType(EInstaMATTextureFileType::InstaMAT_PNG),
	Quality(100u),
	bAllowDithering(true)
	{
	}

	EInstaMATExecutionFormat ExecutionFormat;	/**< The execution format. */
	EInstaMATRotation Rotation;					/**< The texture rotation. */
	EInstaMATTextureSize Width;					/**< The Texture width. */
	EInstaMATTextureSize Height;				/**< The Texture height. */
	EInstaMATTextureFileType FileType;			/**< The output file type. */
	uint32 Quality;								/**< The quality for compressed file types. */
	bool bAllowDithering;						/**< Whether Dithering is enabled. */
};

class IInstaMAT
{
public:
	virtual ~IInstaMAT() {}	
	
	virtual InstaMAT::IInstaMAT* GetInstaMAT() = 0;

	// InstaMAT Interface
	virtual bool Initialize() = 0;
	virtual bool Shutdown() = 0;
	virtual bool InitializePreviewGenerator() = 0;
	virtual bool AllocPackageFromFile(const FString& FilePath) = 0;
	virtual uint32 LoadEnvironmentPackageFromPath(const FString& Path, const bool bIsSystemLibrary) = 0;
	virtual InstaMAT::IGraphPackage* GetPackageFromFilePath(const FString& FilePath) = 0;
	virtual bool DeallocPackage(const FString& FilePath) = 0;
	virtual bool DeallocPackage(InstaMAT::IGraphPackage *const FilePath) = 0;
	virtual bool GetGraphObjectsInPackage(const InstaMAT::IGraphPackage& Package, InstaMAT::IGraphObject*** OutGraphObjects) = 0;
	virtual uint32 RegisterExternalAssetsFolder(const FString& Path) = 0;
	virtual void UnregisterAllExternalAssetsFolder() = 0;

	virtual void ClearUnusedElementExecutionsAndFreeVideoMemory() = 0;
	virtual bool IsElementExecutionAllocatedForKey(const FString& Key) const = 0;
	virtual bool AllocElementExecutionFromTemplate(const FString& Key, const InstaMAT::IGraphTemplate& Template, const InstaMAT::ElementExecutionFlags::Type ExecutionFlag) = 0;
	virtual bool AllocElementExecution(const FString& Key, const FString& GraphID, const InstaMAT::IGraph* Graph, const InstaMAT::ElementExecutionFlags::Type ExecutionFlag) = 0;
	virtual bool DeallocElementExecution(const FString& Key) = 0;
	virtual bool DeallocElementExecutionOnTaskThread(InstaMAT::IElementExecution* const Execution) = 0;
	virtual InstaMAT::IElementExecution* GetElementExecutionForKey(const FString& Key) = 0;
	virtual const InstaMAT::IGraphObject* GetGraphObjectFromPackageWithID(const FString& FilePath, const FString& GraphID) = 0;
	virtual const InstaMAT::IGraphObject* GetGraphObjectWithID(const FString& GraphID) = 0;
	virtual InstaMAT::IImageSampler* GetImageSamplerForOutputParameter(const FString& Key, const uint32 OutputParameter) = 0;
	virtual InstaMAT::IImageSampler* GetImageSamplerForExecutionAndGraphVariable(InstaMAT::IElementExecution* const Execution, const InstaMAT::IGraphVariable* const Variable) = 0;
	virtual bool GetGraphPreview(const FString& GraphID, const FString& PackageFilePath, uint32& OutWidth, uint32& OutHeight, TArray<FColor>& OutData) = 0;
	virtual uint32 GetInstanceSeedForExecution(const FString& Key) = 0;
	virtual bool SetInstanceSeedForExecution(const FString& Key, const uint32& Value) = 0;
	virtual bool SetArithmeticValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const InstaMAT::ArithmeticGraphValue& GraphValue, const bool bGraphVariableRequiresReset) = 0;
	virtual bool SetElementImageValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const uint32 Width, const uint32 Height, const InstaMAT::IImageSampler::ComponentType ComponentType, const InstaMAT::IImageSampler::PixelType PixelType, const TArray<uint8>& TextureData, const bool bIsSRGB) = 0;
	virtual bool SetElementMeshValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, InstaMAT::IGraphMesh& Mesh) = 0;
	virtual bool SetElementEnumValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const uint32 Value) = 0;
	virtual bool SetElementStringValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const FString& Value) = 0;
	virtual bool SetElementExecutionFormat(const FString& Key, const uint32 Width, const uint32 Height, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat) = 0;
	virtual bool SetCompositionGraphSettings(const FString& Key, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat, const float RotationInRadians, const int32 ShiftWidth, const int32 ShiftHeight) = 0;
	virtual bool ExecuteElementExecutionForKey(const FString& Key, bool bIsHighColorDepth, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat, const float RotationInRadians, const int32 ShiftWidth=0, const int32 ShiftHeight=0, float* ProgressReceiver=nullptr) = 0;
	virtual bool GetDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, TArray<uint8>& OutData) = 0;
	virtual bool GetColorSpaceForOutputParameter(const FString& Key, const uint32 ParameterIndex, InstaMAT::IGraphVariable::ColorSpaceType& ColorSpace) = 0;
	virtual bool GetMeshDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, const InstaMAT::IGraphMesh** OutMesh) = 0;
	virtual bool GetSceneDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, const InstaMAT::IGraphScene** OutScene) = 0;
	virtual bool ConvertGraphMeshToMeshDescription(const InstaMAT::IGraphMesh& Mesh, FMeshDescription& MeshDescription) = 0;
	virtual bool ConvertMeshDescriptionToGraphMesh(const struct FMeshDescription& InMesh, InstaMATMesh& OutMesh) = 0;
	virtual void GetInputAndOutputParameterDefinitions(const FString& GraphID, TArray<FInstaMATGraphObjectInputData>& OutInputArray, TArray<FInstaMATGraphObjectOutputData>& OutOutputArray) = 0;
	virtual bool IsPBRMaterialGraph(const InstaMAT::IGraphObject* const GraphObject) = 0;
	virtual bool AllocPackageFromPath(const FString& FilePath) = 0;
	virtual FString GetMachineKeyAsFString() = 0;
	virtual bool IngestLicense(const FString& FilePath) = 0;
	virtual bool IsCachedPreviewImageAvailable(const FString& GraphID) = 0;
	virtual InstaMAT::IInstaMATThread* GetInstaMATThread() = 0;
	virtual bool IsAsyncOperationInProgress() = 0;
	virtual void SetAsyncOperationInProgress(const bool bIsExecuting) = 0;
	virtual float* GetProgressValue() = 0;
	virtual bool TryLoadingPreviewImageFromCache(const FString& GraphID, const FString& PackagePath, uint32& OutWidth, uint32& OutHeight, TArray<FColor>& OutData) = 0;
	virtual int64 GetDefaultVRAMBudget() = 0;

	virtual TTuple<EInstaMATTextureSize, EInstaMATTextureSize> GetDefaultResolutionSettings() const = 0;
	virtual TTuple<EInstaMATTextureSize, EInstaMATTextureSize> GetDefaultPreviewResolutionSettings() const = 0;
	virtual EInstaMATExecutionFormat GetDefaultExecutionFormatSettings() const = 0;

	virtual const TArray<FString>& GetUserCategories(const bool bIsUpdateEnforced) = 0;
	virtual const TArray<FString>& GetCategories(const bool bIsUpdateEnforced) = 0;
	virtual TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> GetGraphObjectLibraryPreviews(bool bEnforceRecache=false) = 0;
	virtual TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> FindGraphObjectWithName(const FString& Name) = 0;
};

class FInstaMAT : public IModuleInterface, public IModularFeature, public IInstaMAT
{
public:
	virtual ~FInstaMAT() { }

	virtual bool Initialize() override;
	virtual bool Shutdown() override;
	virtual bool InitializePreviewGenerator() override;
	virtual bool AllocPackageFromFile(const FString& FilePath) override;
	virtual uint32 LoadEnvironmentPackageFromPath(const FString& Path, const bool bIsSystemLibrary) override;
	virtual InstaMAT::IGraphPackage* GetPackageFromFilePath(const FString& FilePath) override;
	virtual bool DeallocPackage(const FString& FilePath) override;
	virtual bool DeallocPackage(InstaMAT::IGraphPackage *const FilePath) override;
	virtual bool GetGraphObjectsInPackage(const InstaMAT::IGraphPackage& Package, InstaMAT::IGraphObject*** OutGraphObjects) override;
	virtual uint32 RegisterExternalAssetsFolder(const FString& Path) override;
	virtual void UnregisterAllExternalAssetsFolder() override;

	virtual void ClearUnusedElementExecutionsAndFreeVideoMemory() override;
	virtual bool IsElementExecutionAllocatedForKey(const FString& Key) const override;
	virtual bool AllocElementExecutionFromTemplate(const FString& Key, const InstaMAT::IGraphTemplate& Template, const InstaMAT::ElementExecutionFlags::Type ExecutionFlag) override;
	virtual bool AllocElementExecution(const FString& Key, const FString& GraphID, const InstaMAT::IGraph* Graph, const InstaMAT::ElementExecutionFlags::Type ExecutionFlag) override;
	virtual bool DeallocElementExecution(const FString& Key) override;
	virtual bool DeallocElementExecutionOnTaskThread(InstaMAT::IElementExecution* const Execution) override;
	virtual InstaMAT::IElementExecution* GetElementExecutionForKey(const FString& Key) override;
	virtual const InstaMAT::IGraphObject* GetGraphObjectFromPackageWithID(const FString& FilePath, const FString& GraphID) override;
	virtual const InstaMAT::IGraphObject* GetGraphObjectWithID(const FString& GraphID) override;
	virtual InstaMAT::IImageSampler* GetImageSamplerForOutputParameter(const FString& Key, const uint32 OutputParameter) override;
	virtual InstaMAT::IImageSampler* GetImageSamplerForExecutionAndGraphVariable(InstaMAT::IElementExecution* const Execution, const InstaMAT::IGraphVariable* const Variable) override;
	virtual bool GetGraphPreview(const FString& GraphID, const FString& PackageFilePath, uint32& OutWidth, uint32& OutHeight, TArray<FColor>& OutData) override;
	virtual uint32 GetInstanceSeedForExecution(const FString& Key) override;
	virtual bool SetInstanceSeedForExecution(const FString& Key, const uint32& Value) override;
	virtual bool SetElementImageValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const uint32 Width, const uint32 Height, const InstaMAT::IImageSampler::ComponentType ComponentType, const InstaMAT::IImageSampler::PixelType PixelType, const TArray<uint8>& TextureData, const bool bIsSRGB) override;
	virtual bool SetArithmeticValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const InstaMAT::ArithmeticGraphValue& GraphValue, const bool bGraphVariableRequiresReset) override;
	virtual bool SetElementMeshValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, InstaMAT::IGraphMesh& Mesh) override;
	virtual bool SetElementEnumValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const uint32 Value) override;
	virtual bool SetElementStringValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const FString& Value) override;
	virtual bool SetCompositionGraphSettings(const FString& Key, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat, const float RotationInRadians, const int32 ShiftWidth, const int32 ShiftHeight) override;
	virtual bool SetElementExecutionFormat(const FString& Key, const uint32 Width, const uint32 Height, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat) override;
	virtual bool ExecuteElementExecutionForKey(const FString& Key, bool bIsHighColorDepth, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat, const float RotationInRadians, const int32 ShiftWidth=0, const int32 ShiftHeight=0, float* ProgressReceiver = nullptr) override;
	virtual bool GetDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, TArray<uint8>& OutData) override;
	virtual bool GetColorSpaceForOutputParameter(const FString& Key, const uint32 ParameterIndex, InstaMAT::IGraphVariable::ColorSpaceType& ColorSpace) override;
	virtual bool GetMeshDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, const InstaMAT::IGraphMesh** OutMesh) override;
	virtual bool GetSceneDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, const InstaMAT::IGraphScene** OutScene) override;
	virtual bool ConvertGraphMeshToMeshDescription(const InstaMAT::IGraphMesh& Mesh, FMeshDescription& MeshDescription) override;
	virtual bool ConvertMeshDescriptionToGraphMesh(const struct FMeshDescription& InMesh, InstaMATMesh& OutMesh) override;
	virtual void GetInputAndOutputParameterDefinitions(const FString& GraphID, TArray<FInstaMATGraphObjectInputData>& OutInputArray, TArray<FInstaMATGraphObjectOutputData>& OutOutputArray) override;
	virtual bool IsPBRMaterialGraph(const InstaMAT::IGraphObject* const GraphObject) override; 
	virtual bool AllocPackageFromPath(const FString& FilePath) override;
	virtual FString GetMachineKeyAsFString() override;
	virtual bool IngestLicense(const FString& FilePath) override;
	virtual bool IsCachedPreviewImageAvailable(const FString& GraphID) override;
	virtual bool IsAsyncOperationInProgress() override;
	virtual void SetAsyncOperationInProgress(const bool bIsExecuting) override;
	virtual float* GetProgressValue() override;
	virtual bool TryLoadingPreviewImageFromCache(const FString& GraphID, const FString& PackagePath, uint32& OutWidth, uint32& OutHeight, TArray<FColor>& OutData) override;
	virtual int64 GetDefaultVRAMBudget() override;

	virtual TTuple<EInstaMATTextureSize, EInstaMATTextureSize> GetDefaultResolutionSettings() const override;
	virtual TTuple<EInstaMATTextureSize, EInstaMATTextureSize> GetDefaultPreviewResolutionSettings() const override;
	virtual EInstaMATExecutionFormat GetDefaultExecutionFormatSettings() const override;

	virtual const TArray<FString>& GetUserCategories(const bool bIsUpdateEnforced) override;
	virtual const TArray<FString>& GetCategories(const bool bIsUpdateEnforced) override;
	virtual TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> GetGraphObjectLibraryPreviews(bool bEnforceRecache=false) override;
	virtual TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> FindGraphObjectWithName(const FString& Name) override;

	static FInstaMAT* Create(InstaMAT::IInstaMAT* InstaMATAPI)
	{
		return new FInstaMAT(InstaMATAPI);
	}
		
	virtual InstaMAT::IInstaMAT* GetInstaMAT()
	{
		return InstaMAT;
	}

	virtual InstaMAT::IInstaMATThread* GetInstaMATThread()
	{
		return InstaMATThread;
	}

private:
	explicit FInstaMAT(InstaMAT::IInstaMAT *const InstaMATAPI);
	
	/**
	 * Dispatches a system notification.
	 * 
	 * @param InText The text.
	 * @param type The type.
	 */
	void DispatchNotification(const FText& InText, /*SNotificationItem::ECompletionState*/int32 type);
	
	FString VersionString;													/**< The plugin version string. */
	InstaMAT::IInstaMAT* InstaMAT;											/**< InstaMAT API object. */
	InstaMAT::IInstaMATThread* InstaMATThread;								/**< InstaMAT Backend thread. */

	TMap<FString, class InstaMAT::IGraphPackage*> Packages;					/**< All loaded packages. */
	TMap<FString, class InstaMAT::IElementExecution*> ElementExecutions;	/**< All active Element Executions. */

	static bool bIsAsyncProcessRunning;	/**< Whether an Async process is running. */
	
	FCriticalSection Mutex;
};

#endif
