/**
 * InstaMATImporterFactory.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterFactory.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "Factories/SceneImportFactory.h"
#include "InstaMAT/InstaMATEnum.h"
#include "InstaMAT/InstaMATAPI.h"
#include "InstaMATImporterFactory.generated.h"

/**
 * The FInstaMATMaterialParameters struct contains information about the material.
 */
struct INSTAMATIMPORTER_API FInstaMATMaterialParameters
{
	FInstaMATMaterialParameters() = delete;
	FInstaMATMaterialParameters(const float InDisplacementHeight, const float InWidth, const float InHeight, const bool bInHasPhysicalSize, const bool bInIsBaseColorGrayscale) :
	DisplacementHeight(InDisplacementHeight),
	Width(InWidth),
	Height(InHeight),
	bHasPhysicalSize(bInHasPhysicalSize),
	bIsBaseColorGrayscale(bInIsBaseColorGrayscale)
	{
	}

	float DisplacementHeight;	/**< The displacement height. */
	float Width;				/**< The width. */
	float Height;				/**< The height. */
	bool bHasPhysicalSize;		/**< Whether physical size is enabled.*/
	bool bIsBaseColorGrayscale;	/**< Whether the base color is grayscale. */
};

 /**
  * The FInstaMATImporterUtility class contains utility functions to import graph objects.
  */
class INSTAMATIMPORTER_API FInstaMATImporterUtility
{
public:
	/**
	 * Creates a graph instance with the specified parameters.
	 *
	 * @param InstaMATModule the InstaMAT module.
	 * @param Path The path to create the object in.
	 * @param GraphFactory the graph asset.
	 * @param CustomName the name of the new instance.
	 * @return the created asset.
	 */
	static class UInstaMATImporterGraphInstance* CreateGraphInstance(class FInstaMATModule& InstaMATModule, const FString& Path, class UInstaMATImporterGraph* const GraphFactory, const FString& CustomName);
	
	/**
	 * The ImportFile function Creates UObject files derived from the InstaMAT file.
	 *
	 * @param InClass the class
	 * @param InParent the parent
	 * @param InName the Name
	 * @param Filename the file name
	 * @param Flags the object flags
	 * @param Warn warning context
	 * @return An array of created UObjects
	 */
	static TArray<class UInstaMATImporterGraph*> ImportFile(UClass* const InClass, UObject* const InParent, const FName& InName, const FString& Filename, const EObjectFlags Flags, FFeedbackContext* const Warn);
	
	/**
	 * The ImportFileFromGraphObjects function Creates UObject from the specified \p GraphObjects.
	 *
	 * @param GraphObjects			An array of all graph objects to create UObjects from.
	 * @param InClass				The UClass of the objects being created.
	 * @param InParent				The parent object of the created graphs.
	 * @param InName				Name of the file being imported.
	 * @param Filename				The path to the importing file, local to the Contents folder.
	 * @param Flags					The object flags
	 * @param Warn					Warning context.
	 * @param GraphNameOverride		Optional. A custom name to override the imported Graphs with. If multiple graphs are imported, their index will be added to the end of the name.
	 * @return An array of created UObjects
	 */
	static TArray<class UInstaMATImporterGraph*> ImportFileFromGraphObjects(const TArray<const InstaMAT::IGraphObject*>& GraphObjects, UClass* const InClass, UObject* const InParent, const FName& InName, const FString& Filename, const EObjectFlags Flags, FFeedbackContext* const Warn, const FString& GraphNameOverride = FString());
	
	/**
	 * Creates a preview image for the specified instance.
	 *
	 * @param InstaMATModule the InstaMAT module.
	 * @param Parent the parent UE package.
	 * @param PackagePath The InstaMAT Package path (can be empty for environment graphs).
	 * @param GraphID the Graph ID of the graph.
	 * @param bUseAlpha Whether the alpha channel should be used.
	 * @return True upon success.
	 */
	static UTexture2D* CreatePreviewImage(FInstaMATModule& InstaMATModule, UPackage* const Parent, const FString& PackagePath, const FString& Name, const FString& GraphID, const bool bUseAlpha);

	/**
	 * Creates a UTexture2D object from the provided \p ColorData.
	 * 
	 * @param Width The bitmap width.
	 * @param Height The bitmap height.
	 * @param ColorData The color data.
	 * @return The texture.
	 */
	static UTexture2D* CreateTextureFromBitmapData(const uint32 Width, const uint32 Height, const TArray<FColor>& ColorData);

	/**
	 * Creates the Input type for the specified object.
	 *
	 * @param GraphInstance the parent object.
	 * @param VariableType the variable type.
	 * @param InputName the Name.
	 * @return Input Parameter object.
	 */
	static class UInstaMATInputBase* CreateInputObjectForType(class UInstaMATImporterGraphInstance* const GraphInstance, const InstaMAT::IGraphVariable::Type VariableType, const FString& InputName);

	/**
	 * Loads and applies the meta data for the specified \p Input.
	 *
	 * @param Input the input.
	 * @param GraphVariableObject the graph variable source.
	 */
	static void ApplyMetaDataForInputWithGraphVariable(UInstaMATInputBase* const Input, const InstaMAT::IGraphObject* const GraphVariableObject);

	/**
	 * Retrieves the GraphClass representation of the provided \p GraphObject.
	 *
	 * @param InstaMAT the InstaMAT API.
	 * @param GraphObject the graph object.
	 * return the graph class object
	 */
	static const InstaMAT::IGraph* GetGraphClassFromGraphObject(InstaMAT::IInstaMAT* InstaMAT, const InstaMAT::IGraphObject* const GraphObject);

	/**
	 * Imports the Input Parameters for the specified object.
	 *
	 * @param GraphInstance the parent Unreal Engine object.
	 * @param GraphObject the parent InstaMAT object.
	 * @param InstaMATModule the InstaMAT Module.
	 */
	static void ImportInputParametersForGraphObject(UInstaMATImporterGraphInstance* const GraphInstance, const InstaMAT::IGraphObject* const GraphObject, FInstaMATModule& InstaMATModule);

	/**
	 * Imports the Output Texture Parameters for the specified object.
	 *
	 * @param GraphInstance the parent Unreal Engine object.
	 * @param GraphObject the parent InstaMAT object.
	 * @param InstaMATModule the InstaMAT Module.
	 * @param UnrealPackage the Package to safe the UTexture in.
	 */
	static void ImportOutputParametersForGraphObject(UInstaMATImporterGraphInstance* const GraphInstance, const InstaMAT::IGraphObject* const GraphObject, FInstaMATModule& InstaMATModule, UPackage* const UnrealPackage);

	/**
	 * Imports the Output Texture data for the specified object.
	 *
	 * @param GraphInstance the parent Unreal Engine object.
	 * @param GraphObject the parent InstaMAT object.
	 * @param InstaMATModule the InstaMAT Module.
	 * @param UnrealPackage the Package to safe the UTexture in.
	 */
	static void ImportOutputDataForGraphObject(UInstaMATImporterGraphInstance* const GraphInstance, const InstaMAT::IGraphObject* const GraphObject, FInstaMATModule& InstaMATModule, UPackage* const UnrealPackage);
	
	/**
	 * Creates an UMaterialInstance from the InstaMAT base material.
	 * 
	 * @param Package the package.
	 * @param MaterialName The material name.
	 * @return The created UMaterialInstance.
	 */
	static UMaterialInstanceConstant* CreateFlattenMaterialInstance(UPackage* const Package, const FString& MaterialName);

	/**
	 * Creates an UMaterialInstance from the InstaMAT base material.
	 * If \p Textures contains items, the textures will be assigned to the texture slot
	 * based on the texture name.
	 *
	 * @param Package The package.
	 * @param Textures The textures.
	 * @param MaterialName The material name.
	 * @param MaterialSettings material settings.
	 * @return A display ready UMaterialInstance.
	 */
	static UMaterialInstanceConstant* CreateFlattenMaterialInstanceForTextures(UPackage* const Package, TArray<UTexture2D*>& Textures, const FString& MaterialName, const FInstaMATMaterialParameters& MaterialSettings);

	/**
	 * Creates an UMaterialInstance from the InstaMAT base material.
	 * And sets the textures and properties provided to the function.
	 *
	 * @param Package the package.
	 * @param Textures the textures.
	 * @param MaterialName the material name.
	 * @param Properties The material properties.
	 * @return A display ready UMaterialInstance.
	 */
	static UMaterialInstanceConstant* CreateFlattenMaterialInstanceForGraphScene(UPackage* const Package, const TMap<InstaMAT::GraphMaterialTexture::Type, UTexture2D*>& Textures, const FString& MaterialName, const InstaMAT::GraphMaterialProperties& Properties);

	/**
	 * Determines whether \p InputClass keeps track of an arithmetic value.
	 *
	 * @param InputClass the class object of the input.
	 * @return true if it is an arithmetic value.
	 */
	static bool IsInstaMATInputClassArithmetic(UClass* const InputClass);

	/**
	 * Gets the arithmetic value for the \p Input.
	 *
	 * @param Input the input.
	 * @return The arithmetic value.
	 */
	static InstaMAT::ArithmeticGraphValue GetArithmeticValueForInput(UInstaMATInputBase* const Input);

	/**
	 * Sets the arithmetic \p Value for the specified \p Input.
	 *
	 * @param Value the value.
	 * @param Input the input.
	 */
	static void SetArithmeticValueForInput(const InstaMAT::ArithmeticGraphValue& Value, UInstaMATInputBase* const Input);

	/**
	 * Updates the Input texture for the specified \p ElementExecutionKey.
	 * 
	 * @param InstaMATInterface the InstaMAT Module interface.
	 * @param ElementExecutionKey the element execution key.
	 * @param Input the input.
	 * @return True upon success.
	 */
	static bool UpdateInputTexture(class IInstaMAT* const InstaMATInterface, const FString& ElementExecutionKey, class UInstaMATInputBase* const Input);

	/**
	 * Updates the specified \p GraphInstance, by feeding the input values
	 * into InstaMAT and retrieving the new updated texture values.
	 *
	 * @param GraphInstance The graph instance.
	 * @param bOnlyUpdateGPU If enabled only updates the GPU textures.
	 */
	static void UpdateGraphInstance(UInstaMATImporterGraphInstance* const GraphInstance, const bool bOnlyUpdateGPU);

	/**
	 * Ensures that an valid ElementExecution is allocated for the specified \p GraphInstance.
	 *
	 * @param InstaMATInterface The InstaMAT interface.
	 * @param GraphInstance The graph instance.
	 * @param [out] OutbAllocatedNewElementExecution Whether a new execution is allocated.
	 * @return True upon success.
	 */
	static bool EnsureValidElementExecutionForGraphInstanceIsAllocated(IInstaMAT* const InstaMATInterface, const UInstaMATImporterGraphInstance* const GraphInstance, bool& OutbAllocatedNewElementExecution);

	/**
	 * Saves the output images to disk for the graph instance.
	 *
	 * @param GraphInstance the graph.
	 * @param SaveOutputs whether the output should be exported.
	 * @param Directory the save directory.
	 * @param ExportSettings The export settings.
	 */
	static void SaveOutputImagesToDiskForGraphInstance(const UInstaMATImporterGraphInstance* const GraphInstance, const TMap<const class UInstaMATOutput*, bool>& SaveOutputs, const FString& Directory, const struct FInstaMATExportTextureSettings& ExportSettings);

	/**
	 * Updates the \p InputParameters on the execution with the specified \p ExecutionKey.
	 *
	 * @param InstaMATInterface The InstaMAT interface.
	 * @param InputParameters The input parameters.
	 * @param ExecutionKey The execution key.
	 */
	static void UpdateInputParameters(IInstaMAT* const InstaMATInterface, const TArray<UInstaMATInputBase*>& InputParameters, const FString& ExecutionKey);

	/**
	 * Loads the preview texture of the specified \p ViewItem.
	 * 
	 * @param ViewItem the graph library object.
	 * @return True upon success.
	 */
	static bool LoadPreviewTexture(struct FInstaMATGraphObjectViewItem& ViewItem);

	/**
	 * Determines whether a cached preview image exists for
	 * the specified \p GraphID.
	 * 
	 * @param GraphID the graphID.
	 * @return True upon success.
	 */
	static bool IsCachedPreviewImageAvailable(const FString& GraphID);

	/**
	 * Recreates the missing output textures for the specified \p GraphInstance.
	 * 
	 * @param GraphInstance The graph instance.
	 * @return True upon success.
	 */
	static bool RecreateMissingOutputTexturesForGraphInstance(UInstaMATImporterGraphInstance* const GraphInstance);

	/**
	 * Connects the textures to the material.
	 * @param PropertyToTexture The texture to material connection.
	 * @param Material The material.
	 * @param DisplacementHeight The displacement height.
	 * @param bIsBaseColorGrayscale Whether the base color map is grayscale.
	 */
	static void ConnectTexturesToMaterialByProperty(const TMap<EMaterialProperty, UTexture2D*>& PropertyToTexture, UMaterialInstanceConstant* const Material, const float DisplacementHeight, const bool bIsBaseColorGrayscale);

	/**
	 * Connects the textures and the material through static parameters.
	 *
	 * @param Textures The textures.
	 * @param Material The material.
	 * @param MaterialSettings The material settings.
	 */
	static void ConnectTexturesToMaterial(const TArray<UTexture2D*>& Textures, UMaterialInstanceConstant* const Material, const FInstaMATMaterialParameters& MaterialSettings);

	/**
	 * Connects the textures and the material through static parameters.
	 *
	 * @param Textures The textures.
	 * @param Material The material.
	 * @param Properties The material properties.
	 */
	static void ConnectTexturesToMaterial(const TMap<InstaMAT::GraphMaterialTexture::Type, UTexture2D*>& Textures, UMaterialInstanceConstant* const Material, const InstaMAT::GraphMaterialProperties& Properties);

	/**
	 * Updates the resolution values depending on the current resolution.
	 * 
	 * @param [out] OutWidth the input width.
	 * @param [out] OutHeight the input height.
	 * @param [out] OutShiftWidth the input shift.
	 * @param [out] OutShiftHeight the input shift.
	 */
	static void GetResolutionValues(uint32& OutWidth, uint32& OutHeight, int32& OutShiftWidth, int32& OutShiftHeight);

	/**
	 * Ensures that the specified \p ObjectName has only valid characters.
	 * 
	 * @param ObjectName The object name to check.
	 * @return Valid object name.
	 */
	static FString EnsureValidObjectName(const FString& ObjectName);

	/**
	 * Called when the element execution finished on the task thread.
	 *
	 * @param GraphInstance The graph instance.
	 * @param bUpdateResource Update the resources used by the graph instance, otherwise only updates the GPU textures.
	 * @return True upon success.
	 */
	static bool FinishedGraphInstanceExecution(UInstaMATImporterGraphInstance* const GraphInstance, const bool bUpdateResource);

	/**
	 * Imports the output graph scene.
	 * 
	 * @param InstaMATModule The InstaMAT module.
	 * @param GraphInstance The graph instance to update.
	 * @param OutputScene The graph scene output.
	 * @param PackagePath The pacakge path.
	 */
	static void ImportOutputGraphScene(class FInstaMATModule& InstaMATModule, UInstaMATImporterGraphInstance* const GraphInstance, class UInstaMATGraphSceneOutput* const OutputScene, const FString& PackagePath);
};

/**
 * The UInstaMATImporterFactory class implements the UFactory interface. If an InstaMAT file is dragged into the
 * Content Browser an instance of this class will create all necessary objects.
 */
UCLASS(transient)
class INSTAMATIMPORTER_API UInstaMATImporterFactory : public UFactory, public IImportSettingsParser
{
	GENERATED_UCLASS_BODY()

public:

	/**
	 * Creates all objects with the specified parameters. This function is called when using Import function in the Content Browser.
	 * 
	 * @param InClass the class
	 * @param InParent the object parent
	 * @param InName the name of the object
	 * @param Flags the object flags
	 * @param Filename the file name
	 * @param Parms the parms
	 * @param Warn warnings
	 * @param [out] bOutOperationCanceled in case the import process get canceled
	 * @return the imported object 
	 */
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

	/**
	 * Creates all objects with the specified parameters. This function is called when Drag and Dropping into the Content Browser.
	 *
	 * @param InClass the class
	 * @param InParent the object parent
	 * @param InName the name of the object
	 * @param Flags the object flags
	 * @param Type the type
	 * @param buffer the buffer
	 * @param Warn warnings
	 * @return the imported object
	 */
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BUfferEnd, FFeedbackContext* warn) override;

	/**
	 * Checks whether the Filename is valid for import with this factory importer.
	 *
	 * @param Filename the filename
	 * @return true if InstaMAT file.
	 */
	virtual bool FactoryCanImport(const FString& Filename) override;
	
	/**
	 * Finish function after the import process is ended.
	 */
	virtual void CleanUp() override;
	
	/**
	 * Returns the importer settings parser.
	 * 
	 * @return this instance.
	 */
	virtual IImportSettingsParser* GetImportSettingsParser() override { return this; }

	/**
	 * Parses the settings from the json object.
	 *
	 * @param ImportSettingsJson the settings in json format. 
	 */
	virtual void ParseFromJson(TSharedRef<class FJsonObject> ImportSettingsJson) override;

	/**
	 * Parses the \p MetaData String and puts them into a typed array.
	 *
	 * @param MetaData the MetaData.
	 * @return the Array containing the typed values.
	 */
	template<typename T, uint32 IndexCount>
	static TArray<T> ParseToMinimumMaximumValueArray(const FString& MetaData)
	{
		if (MetaData.IsEmpty())
			return TArray<T>();

		TArray<FString> Tokens;
		MetaData.ParseIntoArray(Tokens, TEXT(";"), true);

		if (Tokens.Num() < IndexCount)
			return TArray<T>();

		TArray<T> Values;
		Values.SetNumZeroed(IndexCount);

		for (uint32 Index=0u; Index<IndexCount; Index++)
		{
			const FString& Value = Tokens[Index];
			const float FloatValue = FCString::Atof(*Value);
			if(FloatValue >= (float)TNumericLimits<T>::Max())
			{
				Values[Index] = TNumericLimits<T>::Max();
				continue;
			}
			Values[Index] = T(FloatValue);
		}

		return Values;
	}

	/**
	 * Updates the specified \p GraphInstance, by feeding the input values into
	 * InstaMAT and retrieving the new updated texture values.
	 *
	 * @param GraphInstance The graph instance.
	 * @param bOnlyUpdateGPU If enabled only updates the GPU textures.
	 */
	static void UpdateGraphInstance(class UInstaMATImporterGraphInstance* const GraphInstance, const bool bOnlyUpdateGPU);

	/**
	 * Saves the output images to disk for the graph instance.
	 * 
	 * @param GraphInstance the graph.
	 * @param SaveOutputs whether the output should be exported.
	 * @param Directory the save directory.
	 * @param Type the file type.
	 * @param Rotation the rotation.
	 */
	static void SaveOutputImagesToDiskForGraphInstance(const class UInstaMATImporterGraphInstance* const GraphInstance, const TMap<const class UInstaMATOutput*, bool>& SaveOutputs, const FString& Directory, const FInstaMATExportTextureSettings& ExportSettings);

	/**
	 * Creates an instance for the \p GraphObjectFactory.
	 *
	 * @param GraphObjectFactory	The graph object to create the Instance from.
	 * @param TargetDirectoryPath	Optional. The target folder path to of the new Instance. If empty, Instance will be created in the same folder as the Graph.
	 * @return True upon success.
	 */
	static UInstaMATImporterGraphInstance* CreateGraphInstance(class UInstaMATImporterGraph* const GraphObjectFactory, const FString& TargetDirectoryPath = FString());

	/**
	 * Creates a graph object from the specified \p GraphID.
	 * 
	 * @param GraphID the id.
	 * @return True upon success.
	 */
	static bool ImportGraphObjectWithID(const FString& GraphID);

	/**
	 * Creates a graph instance from the specified \p GraphObject.
	 *
	 * @param GraphObject the graph object.
	 * @param TargetPath Optional target.
	 * @return True upon success.
	 */
	static bool ImportGraphFromGraphObject(const class InstaMAT::IGraphObject* const GraphObject, const FString TargetPath = FString());

	/**
	 * Imports a specified file.
	 *
	 * @param Package the object parent
	 * @param Name the name of the object
	 * @param Filename the file name
	 * @param Flags the file flags
	 * @return The imported objects 
	 */
	static TArray<UInstaMATImporterGraph*> ImportFile(UPackage* const Package, const FName& Name, const FString& Filename, const EObjectFlags Flags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone | EObjectFlags::RF_Transactional);
};