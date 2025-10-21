/**
 * InstaMAT.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMAT.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATPCH.h"
#include "InstaMATModule.h"
#include "InstaMAT/InstaMAT.h"
#include "InstaMAT/InstaMATMesh.h"
#include "InstaMAT/InstaMATSettings.h"

#include "Slate/InstaMATPluginStyle.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#include "MaterialUtilities.h"
#include "StaticMeshOperations.h"
#include "StaticMeshAttributes.h"
#include "MeshAttributes.h"
#include "MeshDescription.h"
#include "MeshDescriptionOperations.h"
#include "Misc/ScopeLock.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"

bool FInstaMAT::bIsAsyncProcessRunning = false;

static const FString kInstaMATUserPackageType(TEXT("UserLibrary"));										/**< UserLibrary default value. */
static const ANSICHAR* kInstaMATMetaDataInputParameterCategoryOrder = "InputParameterCategoryOrder";	/**< Metadata key to retrieve sort order of input categories. */
static const ANSICHAR* kInstaMATMetaDataTags = "Tags";													/**< Metadata key to retrieve tags. */

#define LOCTEXT_NAMESPACE "InstaMAT"

/**
 * The InstaMATMeshUtility has functions for converting
 * meshes and other data structures.
 */
namespace InstaMATMeshUtility
{
	/**
	 * Changes the handedness for \p InVector from Left-Handed to Right-Handed and vice versa.
	 *
	 * @param InVector The vector.
	 * @return Vector with swapped axis.
	 */
	static FORCEINLINE FVector3f InvertVector3Handedness(const FVector3f& InVector)
	{
		static const FMatrix44f kRotationMatrix = FRotationMatrix44f(FRotator3f(0.0f, 0.0f, -90.0f));

		FVector3f Result = InVector;
		Result.Z *= -1;
		Result = kRotationMatrix.TransformVector(Result);
		return Result;
	}

	/**
	 * Converts a InstaMAT graph vector3 object into a Unreal Engine vector3 object.
	 *
	 * @param MatVector							The InstaMAT vector to transform from.
	 * @param bIsConvertingToUEHandedness		Optional. If true, the vector will be converted into Unreal Engine coordinate system and if false, the returned vector
	 *											will have the same values as the input vector.
	 */
	static FORCEINLINE FVector3f MatVector3FToUEVector3F(const InstaMAT::GraphVec3F& MatVector, bool bIsConvertingToUEHandedness = true)
	{
		if (bIsConvertingToUEHandedness)
			return InvertVector3Handedness(FVector3f(MatVector.X, MatVector.Y, MatVector.Z));

		return FVector3f(MatVector.X, MatVector.Y, MatVector.Z);
	}

	/**
	 * Converts a Unreal Engine Vector3 object into a InstaMAT graph Vector3 object.
	 * 
	 * @param UVector							The UE vector to transform from.
	 * @param bIsConvertingToMatHandedness		Optional. If true, the vector will be converted into InstaMAT coordinate system and if false, the returned vector
	 *											will have the same values as the input vector.
	 */
	static FORCEINLINE InstaMAT::GraphVec3F UEVector3FToMatVector3F(const FVector3f& UVector, bool bIsConvertingToMatHandedness = true)
	{
		if (bIsConvertingToMatHandedness)
		{
			const FVector3f Result = InvertVector3Handedness(UVector);
			return InstaMAT::GraphVec3F(Result.X, Result.Y, Result.Z);
		}

		return InstaMAT::GraphVec3F(UVector.X, UVector.Y, UVector.Z);
	}

	/**
	 * Converts GraphVec2F to FVector2D.
	 *
	 * @param Vector InstaMAT GraphVec2F
	 * @return Unreal Engine FVector2f
	 */
	static FORCEINLINE FVector2f GraphVec2FToFVector2f(const InstaMAT::GraphVec2F& Vector)
	{
		return FVector2f(Vector.X, Vector.Y);
	}

	/**
	 * Converts ColorRGBAUI8 to FColor.
	 *
	 * @param Vector InstaMAT GraphVec3F
	 * @return Unreal Engine FVector
	 */
	static FORCEINLINE FColor ColorRGBAUI8ToFColor(const InstaMAT::ColorRGBAUI8& Color)
	{
		return FColor(Color.R, Color.G, Color.B, Color.A);
	}

	/**
	 * Sanitizes an array, checks for NaNs and Infs.
	 *
	 * @param [out] OutData the out data.
	 * @param NumElements the number of elements in the array.
	 * @param InDefaultValue the default value to replace invalid values.
	 */
	static FORCEINLINE void SanitizeFloatArray(float* const OutData, const size_t NumElements, const float InDefaultValue)
	{
		// NOTE: it is possible that Unreal Engine sends invalid data to InstaMAT that contains NaNs or infinite numbers
		// in order to avoid invalid meshes we sanitze all float arrays by default
		for (size_t Index=0; Index<NumElements; Index++)
		{
			if (FMath::IsNaN(OutData[Index]) || !FMath::IsFinite(OutData[Index]))
			{
				UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Sanitzing NaN/infinite value in data received from Unreal Engine."));
				OutData[Index] = InDefaultValue;
			}
		}
	}

	/**
	 * Convertes a GraphMesh to FMeshDescription
	 *
	 * @param SourceGraphMesh the source mesh graph.
	 * @param TargetmeshDescription the target mesh description.
	 */
	static void GraphMeshToMeshDescription(const InstaMAT::IGraphMesh& SourceGraphMesh, FMeshDescription& DestinationMeshDescription)
	{
		DestinationMeshDescription.Empty();

		if (SourceGraphMesh.GetIndexCount() == 0u || SourceGraphMesh.GetVertexCount() == 0u)
			return;

		const uint64 IndexCount = SourceGraphMesh.GetIndexCount();
		const uint64 VertexCount = SourceGraphMesh.GetVertexCount();
		const uint32* const Indices = SourceGraphMesh.GetIndices();
		const uint32* const MaterialIndices = SourceGraphMesh.GetMaterialIndices();
		const InstaMAT::GraphMeshVertex* const Vertices = SourceGraphMesh.GetVertices();
		const uint64 TriangleCount = SourceGraphMesh.GetPolygonCount();

		// Preallocate mesh description data
		DestinationMeshDescription.ReserveNewVertices(VertexCount);
		DestinationMeshDescription.ReserveNewVertexInstances(IndexCount);
		DestinationMeshDescription.ReserveNewPolygons(TriangleCount);
		DestinationMeshDescription.ReserveNewEdges(TriangleCount * 2.5f); // approx.
		DestinationMeshDescription.PolygonGroupAttributes().RegisterAttribute<FName>(MeshAttribute::PolygonGroup::ImportedMaterialSlotName, 1, FName(TEXT("InstaMAT_Material")), EMeshAttributeFlags::Mandatory);

		// Array references for attributes
		TVertexAttributesRef<FVector3f> VertexPositions = DestinationMeshDescription.VertexAttributes().GetAttributesRef<FVector3f>(MeshAttribute::Vertex::Position);
		TVertexInstanceAttributesRef<FVector3f> VertexInstanceNormals = DestinationMeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector3f>(MeshAttribute::VertexInstance::Normal);
		TVertexInstanceAttributesRef<FVector3f> VertexInstanceTangents = DestinationMeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector3f>(MeshAttribute::VertexInstance::Tangent);
		TVertexInstanceAttributesRef<float> VertexInstanceBinormalSigns = DestinationMeshDescription.VertexInstanceAttributes().GetAttributesRef<float>(MeshAttribute::VertexInstance::BinormalSign);
		TVertexInstanceAttributesRef<FVector4f> VertexInstanceColors = DestinationMeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector4f>(MeshAttribute::VertexInstance::Color);
		TVertexInstanceAttributesRef<FVector2f> VertexInstanceUVs = DestinationMeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector2f>(MeshAttribute::VertexInstance::TextureCoordinate);
		TPolygonGroupAttributesRef<FName> PolygonGroupImportedMaterialSlotNames = DestinationMeshDescription.PolygonGroupAttributes().GetAttributesRef<FName>(MeshAttribute::PolygonGroup::ImportedMaterialSlotName);
		
		// Set vertex positions and create map to get from vertex index to vertexID
		TMap<uint32, FVertexID> VertexIndexToVertexID;
		VertexIndexToVertexID.Reserve(VertexCount);

		for (uint32 VertexIndex = 0u; VertexIndex < VertexCount; VertexIndex++)
		{
			const FVertexID VertexId = DestinationMeshDescription.CreateVertex();
			const FVector3f Position = InstaMATMeshUtility::MatVector3FToUEVector3F(Vertices[VertexIndex].Position);

			VertexPositions.Set(VertexId, Position);
			VertexIndexToVertexID.Add(VertexIndex, VertexId);
		}

		// Create Polygongroups from material indices
		for (uint32 MaterialIndex = 0u; MaterialIndex < TriangleCount; MaterialIndex++)
		{
			const FPolygonGroupID MaterialGroupID(MaterialIndices[MaterialIndex]);

			if (DestinationMeshDescription.IsPolygonGroupValid(MaterialGroupID))
				continue;

			DestinationMeshDescription.CreatePolygonGroupWithID(MaterialGroupID);
			PolygonGroupImportedMaterialSlotNames[MaterialGroupID] = FName(FString::Printf(TEXT("InstaMAT_Material_%u"), MaterialIndex));
		}

		/// the fnCalculateBinormalSign lambda calculates the binormal sign
		const auto fnCalculateBinormalSign = [](const FVector3f& Normal, const FVector3f& Binormal, const FVector3f& Tangent) -> float
		{
			const FVector3f CrossTangent = FVector3f::CrossProduct(Binormal, Normal);
			return FVector3f::DotProduct(Tangent, CrossTangent) < 0 ? -1.0f : 1.0f;
		};

		/// The fnIsDegenerateTriangle lambda checks if a triangle has the same vertex multiple times
		const auto fnIsDegenerateTriangle = [&Indices](const uint32 VertexFaceIndexBasis) -> bool
		{
			uint32 VertexIDs[3u];

			for (uint32 CornerIndex = 0u; CornerIndex < 3u; CornerIndex++)
			{
				VertexIDs[CornerIndex] = Indices[VertexFaceIndexBasis + CornerIndex];
			}

			return VertexIDs[0] == VertexIDs[1] || VertexIDs[0] == VertexIDs[2] || VertexIDs[1] == VertexIDs[2];
		};

		bool bIsTangentsInvalid = false;

		// NOTE: InstaMAT has a different handedness, the coordinate system is transformed.
		// Additionally the UV V channel is inverted.

		// Build triangles 
		for (uint32 TriangleIndex = 0u; TriangleIndex < TriangleCount; TriangleIndex++)
		{
			const uint32 VertexFaceIndexBasis = TriangleIndex * 3u;

			// Determine whether degenerates are present
			if (fnIsDegenerateTriangle(VertexFaceIndexBasis))
				continue;

			const FPolygonGroupID MaterialGroupID(MaterialIndices[TriangleIndex]);
			TArray<FVertexInstanceID> TriangleVertexInstanceIDs;
			TriangleVertexInstanceIDs.SetNum(3u);

			// Set vertex attributes
			for (uint32 CornerIndex = 0u; CornerIndex < 3u; CornerIndex++)
			{
				const uint32 VertexPositionIndex = Indices[VertexFaceIndexBasis + CornerIndex];
				const FVertexID VertexID = VertexIndexToVertexID[VertexPositionIndex];
				const FVertexInstanceID VertexInstanceID = DestinationMeshDescription.CreateVertexInstance(VertexID);
				const InstaMAT::GraphMeshVertex& InstaMATVertex = Vertices[VertexPositionIndex];

				const FVector3f Normal = InstaMATMeshUtility::MatVector3FToUEVector3F(InstaMATVertex.Normal);
				const FVector3f Tangent = InstaMATMeshUtility::MatVector3FToUEVector3F(InstaMATVertex.Tangent);
				const FVector3f Binormal = InstaMATMeshUtility::MatVector3FToUEVector3F(InstaMATVertex.Binormal, /*bIsConvertingToUEHandedness:*/false);

				TriangleVertexInstanceIDs[CornerIndex] = VertexInstanceID;
				VertexInstanceNormals[VertexInstanceID] = Normal;
				VertexInstanceTangents[VertexInstanceID] = Tangent;
				VertexInstanceBinormalSigns[VertexInstanceID] = fnCalculateBinormalSign(Normal, Binormal, Tangent);
				VertexInstanceColors[VertexInstanceID] = FLinearColor::FromSRGBColor(ColorRGBAUI8ToFColor(InstaMATVertex.Color));
				VertexInstanceUVs[VertexInstanceID] = FVector2f(InstaMATVertex.TexCoord.X, 1.0f - InstaMATVertex.TexCoord.Y);

				if (Tangent.IsNearlyZero() || Tangent.ContainsNaN())
				{
					bIsTangentsInvalid = true;
				}
			}

			TArray<FVertexInstanceID> TriangleVertexList;
			TriangleVertexList.Add(TriangleVertexInstanceIDs[0]);
			TriangleVertexList.Add(TriangleVertexInstanceIDs[1]);
			TriangleVertexList.Add(TriangleVertexInstanceIDs[2]);
			DestinationMeshDescription.CreateTriangle(MaterialGroupID, TriangleVertexList);
		}

		TArray<uint32> SmoothingGroupArray;
		SmoothingGroupArray.Init(1u, TriangleCount);

		FStaticMeshOperations::ComputeTriangleTangentsAndNormals(DestinationMeshDescription);

		FStaticMeshAttributes Attributes(DestinationMeshDescription);
		Attributes.Register(/*bKeepExistingAttribute:*/ true);

#if defined (INSTAMAT_ENABLE_EDGE_HARDNESS)
		FStaticMeshOperations::ConvertSmoothGroupToHardEdges(SmoothingGroupArray, DestinationMeshDescription);
		FStaticMeshOperations::DetermineEdgeHardnessesFromVertexInstanceNormals(DestinationMeshDescription);
#endif

		if (bIsTangentsInvalid)
		{
			FStaticMeshOperations::ComputeTangentsAndNormals(DestinationMeshDescription, EComputeNTBsFlags::BlendOverlappingNormals | EComputeNTBsFlags::IgnoreDegenerateTriangles | EComputeNTBsFlags::Tangents | EComputeNTBsFlags::UseMikkTSpace);
		}
	}

	/**
	 * Converts a FMeshDescription to GraphMesh.
	 *
	 * @param SourceMeshDescription the source mesh.
	 * @param DestinationGraphMesh the InstaMAT mesh.
	 */
	static void MeshDescriptionToGraphMesh(const FMeshDescription& SourceMeshDescription, InstaMATMesh& DestinationGraphMesh)
	{
		const FStaticMeshConstAttributes SourceMeshAttributes(SourceMeshDescription);
		TVertexAttributesConstRef<FVector3f> VertexPositions = SourceMeshAttributes.GetVertexPositions();
		TVertexInstanceAttributesConstRef<FVector3f> VertexInstanceNormals = SourceMeshAttributes.GetVertexInstanceNormals();
		TVertexInstanceAttributesConstRef<FVector3f> VertexInstanceTangents = SourceMeshAttributes.GetVertexInstanceTangents();
		TVertexInstanceAttributesConstRef<float> VertexInstanceBinormalSigns = SourceMeshAttributes.GetVertexInstanceBinormalSigns();
		TVertexInstanceAttributesConstRef<FVector4f> VertexInstanceColors = SourceMeshAttributes.GetVertexInstanceColors();
		TVertexInstanceAttributesConstRef<FVector2f> VertexInstanceUVs = SourceMeshAttributes.GetVertexInstanceUVs();
		TPolygonGroupAttributesConstRef<FName> PolygonGroupMaterialSlotName = SourceMeshAttributes.GetPolygonGroupMaterialSlotNames();

		if (!VertexPositions.IsValid())
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: The provided mesh does not contain valid vertex positions."));
			return;
		}

		uint32 TriangleCount = 0u;
		for (const FPolygonID& PolygonID : SourceMeshDescription.Polygons().GetElementIDs())
		{
			TriangleCount += SourceMeshDescription.GetNumPolygonTriangles(PolygonID);
		}

		TArray<uint32>& IndicesArray = DestinationGraphMesh.GetIndicesArray();
		TArray<uint32>& SubmeshIndicesArray = DestinationGraphMesh.GetSubmeshIndicesArray();
		TArray<InstaMAT::GraphMeshVertex>& VertexArray = DestinationGraphMesh.GetVerticesArray();
		TArray<uint32>& MaterialIndicesArray = DestinationGraphMesh.GetMaterialIndicesArray();

		const uint32 IndexCount = TriangleCount * 3u;
		SubmeshIndicesArray.SetNum(TriangleCount);
		MaterialIndicesArray.SetNum(TriangleCount);
		IndicesArray.SetNum(IndexCount);
		VertexArray.SetNum(IndexCount); // NOTE: we will shrink it down later

		// Determine whether mesh has vertex colors
		const bool bHasVertexColors = FStaticMeshOperations::HasVertexColor(SourceMeshDescription);

		/// The fnFVector4fToColorRGBAUI8 lambda creates a ColorRGBAUI8 from a FVector4f
		const auto fnFVector4fToColorRGBAUI8 = [](const FVector4f& Value) -> InstaMAT::ColorRGBAUI8
		{
			return InstaMAT::ColorRGBAUI8(Value.X * 255.0f, Value.Y * 255.0f, Value.Z * 255.0f, Value.W * 255.0f);
		};

		if (!VertexInstanceNormals.IsValid())
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: The provided mesh does not contain valid normals."));
		}

		if (!VertexInstanceTangents.IsValid())
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: The provided mesh does not contain valid tangents."));
		}

		if (!VertexInstanceBinormalSigns.IsValid())
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: The provided mesh does not contain valid binormal signs."));
		}

		if (!VertexInstanceUVs.IsValid())
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: The provided mesh does not contain valid UVs."));
		}

		TSet<FVertexInstanceID> UniqueInstances;
		uint32 TriangleIndex = 0u;
		uint32 Index = 0u;

		// NOTE: InstaMAT has a different handedness, the coordinate system is transformed.
		// Additionally the UV V channel is inverted.

		for (const FPolygonID& PolygonID : SourceMeshDescription.Polygons().GetElementIDs())
		{
			const FPolygonGroupID& PolygonGroupID = SourceMeshDescription.GetPolygonPolygonGroup(PolygonID);
			const TArrayView<const FTriangleID>& TriangleIDs = SourceMeshDescription.GetPolygonTriangles(PolygonID);
			const int32 MaterialGroupIndex = PolygonGroupID.GetValue();

			if (!TriangleIDs.IsEmpty())
			{
				for (const FTriangleID& TriangleID : TriangleIDs)
				{
					SubmeshIndicesArray[TriangleIndex] = MaterialGroupIndex;
					MaterialIndicesArray[TriangleIndex] = MaterialGroupIndex;
					uint32 TriangleIndices[3u];

					for (uint32 TriangleCorner = 0u; TriangleCorner < 3u; TriangleCorner++)
					{
						const FVertexInstanceID VertexInstanceID = SourceMeshDescription.GetTriangleVertexInstance(TriangleID, TriangleCorner);
						const int32 VertexIndex = VertexInstanceID.GetValue();
						TriangleIndices[TriangleCorner] = VertexIndex;

						InstaMAT::GraphMeshVertex& Vertex = VertexArray[VertexIndex];
						Vertex.Position = InstaMATMeshUtility::UEVector3FToMatVector3F(VertexPositions.Get(SourceMeshDescription.GetVertexInstanceVertex(VertexInstanceID)));

						if (VertexInstanceUVs.IsValid())
						{
							const FVector2f& UV = VertexInstanceUVs.Get(VertexInstanceID);
							Vertex.TexCoord = InstaMAT::GraphVec2F(UV.X, 1.0f - UV.Y);
						}
						else
						{
							Vertex.TexCoord = InstaMAT::GraphVec2F(0.0f, 0.0f);
						}

						if (VertexInstanceNormals.IsValid())
						{
							const FVector3f Normal = InstaMATMeshUtility::InvertVector3Handedness(VertexInstanceNormals.Get(VertexInstanceID));
							Vertex.Normal = InstaMATMeshUtility::UEVector3FToMatVector3F(Normal, /*bIsConvertingToMatHandedness:*/false);

							if (VertexInstanceTangents.IsValid())
							{
								const FVector3f Tangent = InstaMATMeshUtility::InvertVector3Handedness(VertexInstanceTangents.Get(VertexInstanceID));
								Vertex.Tangent = InstaMATMeshUtility::UEVector3FToMatVector3F(Tangent, /*bIsConvertingToMatHandedness:*/false);

								if (VertexInstanceBinormalSigns.IsValid())
								{
									Vertex.Binormal = InstaMATMeshUtility::UEVector3FToMatVector3F(FVector3f::CrossProduct(Tangent, Normal).GetSafeNormal() * -VertexInstanceBinormalSigns[VertexInstanceID], /*bIsConvertingToMatHandedness:*/false);
								}
								else
								{
									Vertex.Binormal = InstaMAT::GraphVec3F(0.0f, 1.0f, 0.0f);
								}
							}
							else
							{
								Vertex.Tangent = InstaMAT::GraphVec3F(1.0f, 0.0f, 0.0f);
								Vertex.Binormal = InstaMAT::GraphVec3F(0.0f, 1.0f, 0.0f);
							}
						}
						else
						{
							Vertex.Normal = InstaMAT::GraphVec3F(0.0f, 0.0f, 1.0f);
							Vertex.Tangent = InstaMAT::GraphVec3F(1.0f, 0.0f, 0.0f);
							Vertex.Binormal = InstaMAT::GraphVec3F(0.0f, 1.0f, 0.0f);
						}

						Vertex.Color = bHasVertexColors ? fnFVector4fToColorRGBAUI8(VertexInstanceColors.Get(VertexInstanceID)) : InstaMAT::ColorRGBAUI8(0, 0, 0, 0);

						UniqueInstances.Add(VertexInstanceID);
						Index++;
					}

					// Order
					IndicesArray[Index - 3u] = TriangleIndices[0u];
					IndicesArray[Index - 2u] = TriangleIndices[1u];
					IndicesArray[Index - 1u] = TriangleIndices[2u];

					TriangleIndex++;
				}
			}
		}

		VertexArray.SetNum(UniqueInstances.Num());
	}
};

/**
 * The InstaMATGraphVariableUtility has functions for inspecting
 * InstaMAT GraphVariables.
 */
namespace InstaMATGraphVariableUtility
{
	/**
	 * Determines whether the type is an image type.
	 * 
	 * @param Type the type.
	 * @return Whether type is image type.
	 */
	static bool IsImageType(const InstaMAT::IGraphVariable::Type Type)
	{
		return (Type == InstaMAT::IGraphVariable::TypeAtomOutputImage ||
				Type == InstaMAT::IGraphVariable::TypeAtomOutputImageGray ||
				Type == InstaMAT::IGraphVariable::TypeElementImage ||
				Type == InstaMAT::IGraphVariable::TypeElementImageGray);
	}
}

/**
 * The InstaMATCacheFilePathUtility has cache file path
 * related functions.
 */
namespace InstaMATCacheFilePathUtility
{
	/**
	 * Creates the cache file path.
	 * 
	 * @param Path the cache parent directory.
	 * @param GraphID the Graph ID.
	 * @return The combined cache file path.
	 */
	static FString CreateCachePath(const FString& Path, const FString& GraphID)
	{
		check(!Path.IsEmpty() && !GraphID.IsEmpty());

		FString FilePath = FPaths::Combine(Path, TEXT("Cache"));
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// Ensure that the directory exists
		if (!PlatformFile.DirectoryExists(*FilePath))
		{
			if (!PlatformFile.CreateDirectory(*FilePath))
			{
				UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Failed to create File path in InstaMAT user directory='%s'."), *FilePath);
				return FString();
			}
		}

		FilePath = FPaths::Combine(FilePath, FString::Printf(TEXT("%s.png"), *GraphID));
		FPaths::NormalizeFilename(FilePath);
		return FilePath;
	};
	/**
	 * Retrieves the Default environment path from the settings.
	 *
	 * @return The default environment path.
	 */
	static FString GetEnvironmentPath()
	{
		// Get default user path
		UInstaMATSettings* const UserSettings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
		UserSettings->LoadConfig();
		UserSettings->EnsureDefaultUserPathIsSet();
		UserSettings->SaveConfig();
		
#if PLATFORM_MAC
		return FPaths::Combine(*UserSettings->EnvironmentFolder, TEXT("Contents/Resources"));
#endif
		
		return UserSettings->EnvironmentFolder;
	}

	/**
	 * Retrieves the Default user path from the settings.
	 *
	 * @return The default user path.
	 */
	static FString GetDefaultUserPath()
	{
		// Get default user path
		UInstaMATSettings* const UserSettings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
		UserSettings->LoadConfig();
		UserSettings->EnsureDefaultUserPathIsSet();
		UserSettings->SaveConfig();

		FInstaMATUserDirectory* const DefaultUserDirectory = UserSettings->UserFolders.FindByPredicate([](const FInstaMATUserDirectory& Directory)
			{
				return !Directory.UserPath.Path.IsEmpty() && Directory.bIsDefault;
			});

		if (DefaultUserDirectory == nullptr)
			return FString();

		return DefaultUserDirectory->UserPath.Path;
	}
}

/**
 * The InstaMATGraphUtility has functions for inspecting graphs. 
 */
namespace InstaMATGraphUtility
{
	/**
	 * Returns whether the specified \p GraphType is valid in the Unreal Engine context.
	 * Usually useful for checking if a specific graph should be displayed in the UI or not.
	 * 
	 * @param GraphType A graph type to check if its valid or not.
	 * @return true if valid.
	 */
	static inline bool IsGraphTypeValid(const InstaMAT::IGraph::GraphTypePublic& GraphType)
	{
		return (GraphType != (InstaMAT::IGraph::GraphTypePublic)0u) && GraphType != InstaMAT::IGraph::GraphTypeFunction;
	}

	/**
	 * Returns the type of the graph, regardless of its underlying type (graph or template).
	 * 
	 * @param GraphObject Object to get the type from. Can't be nullptr.
	 * @return The type of the graph, or type 0 if class is invalid.
	 */
	static inline InstaMAT::IGraph::GraphTypePublic GetGraphTypeFromGraphObject(const InstaMAT::IGraphObject* const GraphObject)
	{
		check(GraphObject != nullptr);

		InstaMAT::IGraph::GraphTypePublic GraphType = (InstaMAT::IGraph::GraphTypePublic)0u;
		if (const InstaMAT::IGraph* const Graph = GraphObject->AsGraph())
		{
			GraphType = (InstaMAT::IGraph::GraphTypePublic)((InstaMAT::IGraph::GraphTypePublic)Graph->GetGraphTypeUI32() == InstaMAT::IGraph::GraphTypePublic::GraphTypeLazy ? Graph->GetLazyGraphTypeUI32() : Graph->GetGraphTypeUI32());
		}
		else if (const InstaMAT::IGraphTemplate* const TemplateGraph = GraphObject->AsTemplate())
		{
			GraphType = (InstaMAT::IGraph::GraphTypePublic)(TemplateGraph->GetClassGraphTypeUI32());
		}

		return GraphType;
	}

	/**
	 * Determines whether the specified \p GraphObject is valid in our context.
	 *
	 * @param GraphObject the graph object.
	 * @return true if valid.
	 */
	static inline bool IsGraphObjectValid(const InstaMAT::IGraphObject* const GraphObject)
	{
		if (GraphObject == nullptr)
			return false;

		if (GraphObject->ContainsMetaDataKeyChar("IsPrivate") &&
			GraphObject->GetMetaDataAsCharBoolean("IsPrivate"))
			return false;

		const InstaMAT::IGraph::GraphTypePublic GraphType = GetGraphTypeFromGraphObject(GraphObject);
		return IsGraphTypeValid(GraphType);
	}
}

/**
 * The InstaMATThreadStructs contains structs used for the InstaMAT thread.
 */
namespace InstaMATThreadStructs
{
	/**
	 * The PreviewSamplerAllocation contains all information
	 * for retrieving the preview image sampler of a IGraphObject.
	 */
	struct PreviewSamplerAllocation
	{
		PreviewSamplerAllocation(IInstaMAT* const InInstaMATModuleInterface, const FString& InGraphID, FString const InPackagePath) : 
		InstaMATModuleInterface(InInstaMATModuleInterface),
		PackagePath(InPackagePath),
		GraphID(InGraphID),
		OutSampler(nullptr)
		{
			check(InstaMATModuleInterface != nullptr);
			check(!GraphID.IsEmpty());
		}
		
		IInstaMAT* InstaMATModuleInterface;		/**< The InstaMAT module interface object. */
		FString PackagePath;					/**< The Package of the graph. (Can be empty for environment packages.)*/
		FString GraphID;						/**< The Graph ID. */
		InstaMAT::IImageSampler* OutSampler;	/**< The retrieved sampler. */
	};

	/**
	 * The BackendInitialization struct contains
	 * all necessary information to set the
	 * initialize the InstaMAT compute backend on 
	 * the InstaMAT thread.
	 */
	struct BackendInitialization
	{
		BackendInitialization(InstaMAT::IInstaMAT* const InstaMAT, const InstaMAT::IInstaMAT::BackendType InitializationTarget) :
		InstaMATAPI(InstaMAT),
		Target(InitializationTarget),
		InitializationResult(InstaMAT::IInstaMAT::BackendType::BackendTypeNone)
		{
			check(InstaMATAPI != nullptr);
			check(Target != InstaMAT::IInstaMAT::BackendType::BackendTypeNone);
		}

		InstaMAT::IInstaMAT* InstaMATAPI;						/**< The InstaMAT API object. */
		InstaMAT::IInstaMAT::BackendType Target;				/**< The Initialization target. */
		InstaMAT::IInstaMAT::BackendType InitializationResult;	/**< The initialized backend. */
	};

	/**
	 * The ElementExecutionSetFormat struct contains
	 * all necessary information to set the 
	 * Execution format on the InstaMAT thread.
	 */
	struct ElementExecutionSetFormat
	{
		ElementExecutionSetFormat(InstaMAT::IElementExecution* const Execution, const uint32 InWidth, const uint32 InHeight, const InstaMAT::ElementExecutionFormat::Type InFormat) :
		ElementExecution(Execution),
		Width(InWidth),
		Height(InHeight),
		Format(InFormat)
		{
			check(ElementExecution != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;	/**< The element execution. */
		uint32 Width;									/**< The Width. */
		uint32 Height;									/**< The Height. */
		InstaMAT::ElementExecutionFormat::Type Format;	/**< The Format. */
	};

	/**
	 * The ElementExecutionSetImageSampler struct contains
	 * all necessary information to set an
	 * IImageSampler on the InstaMAT thread.
	 */
	struct ElementExecutionSetImageSampler
	{
		ElementExecutionSetImageSampler(InstaMAT::IElementExecution* const Execution, InstaMAT::IGraphVariable* const InVariable, InstaMAT::IImageSampler* const InSampler, const bool bSRGB) :
		ElementExecution(Execution),
		Variable(InVariable),
		Sampler(InSampler),
		bIsSRGB(bSRGB)
		{
			check(ElementExecution != nullptr);
			check(Variable != nullptr);
			check(Sampler != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;	/**< The element execution. */
		InstaMAT::IGraphVariable* Variable;				/**< The variable. */
		InstaMAT::IImageSampler* Sampler;				/**< The sampler. */
		bool bIsSRGB;									/**< Whether the sampler is SRGB. */
	};

	/**
	 * The ElementExecutionSetMesh struct contains
	 * all necessary information to set an
	 * IGraphMesh on the InstaMAT thread.
	 */
	struct ElementExecutionSetMesh
	{
		ElementExecutionSetMesh(InstaMAT::IElementExecution* const Execution, InstaMAT::IGraphVariable* const InVariable, InstaMAT::IGraphMesh* const InMesh) :
		ElementExecution(Execution),
		Variable(InVariable),
		Mesh(InMesh)
		{
			check(ElementExecution != nullptr);
			check(Variable != nullptr);
			check(Mesh != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;	/**< The element execution. */
		InstaMAT::IGraphVariable* Variable;				/**< The variable. */
		InstaMAT::IGraphMesh* Mesh;						/**< The mesh. */
	};

	/**
	 * The ElementExecutionAllocImageSampler struct contains
	 * all necessary information to allocate an
	 * IImageSampler on the InstaMAT thread.
	 */
	struct ElementExecutionAllocImageSampler
	{
		ElementExecutionAllocImageSampler(InstaMAT::IElementExecution* const Execution, const InstaMAT::IGraphVariable* const InVariable) :
		ElementExecution(Execution),
		Variable(InVariable),
		OutSampler(nullptr)
		{
			check(ElementExecution != nullptr);
			check(Variable != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;	/**< The element execution. */
		const InstaMAT::IGraphVariable* Variable;		/**< The variable. */
		InstaMAT::IImageSampler* OutSampler;			/**< The allocated sampler. */
	};

	/**
	 * The ElementExecutionGetMesh struct contains
	 * all necessary information to get a mesh
	 * on the InstaMAT thread.
	 */
	struct ElementExecutionGetMesh
	{
		ElementExecutionGetMesh(InstaMAT::IElementExecution* const Execution, InstaMAT::IGraphVariable* const InVariable) :
		ElementExecution(Execution),
		Variable(InVariable),
		OutMesh(nullptr)
		{
			check(ElementExecution != nullptr);
			check(Variable != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;	/**< The element execution. */
		InstaMAT::IGraphVariable* Variable;				/**< The variable. */
		const InstaMAT::IGraphMesh* OutMesh;			/**< The retrieved mesh. */
	};

	/**
	 * The ElementExecutionGetScene struct contains
	 * all necessary information to get a scene
	 * on the InstaMAT thread.
	 */
	struct ElementExecutionGetScene
	{
		ElementExecutionGetScene(InstaMAT::IElementExecution* const Execution, InstaMAT::IGraphVariable* const InVariable) :
		ElementExecution(Execution),
		Variable(InVariable),
		OutScene(nullptr)
		{
			check(ElementExecution != nullptr);
			check(Variable != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;	/**< The element execution. */
		InstaMAT::IGraphVariable* Variable;				/**< The variable. */
		const InstaMAT::IGraphScene* OutScene;			/**< The retrieved scene. */
	};

	/**
	 * The ElementExecutionDealloc struct contains
	 * all necessary information to dealloc an
	 * execution on the InstaMAT thread.
	 */
	struct ElementExecutionDealloc
	{
		ElementExecutionDealloc(InstaMAT::IElementExecution* const Execution, InstaMAT::IInstaMAT* const InstaMAT) :
		ElementExecution(Execution),
		InstaMATAPI(InstaMAT)
		{
			check(ElementExecution != nullptr);
			check(InstaMATAPI != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;	/**< The element execution. */
		InstaMAT::IInstaMAT* InstaMATAPI;				/**< The InstaMAT API. */
	};

	/**
	 * The ElementExecutionExecute struct contains
	 * all necessary information to execute an
	 * ElementExecution on the InstaMAT thread.
	 */
	struct ElementExecutionExecute 
	{
		ElementExecutionExecute(InstaMAT::IElementExecution* const Execution, float* const InProgressReceiver = nullptr) :
		ElementExecution(Execution),
		ProgressReceiver(InProgressReceiver)
		{
			check(ElementExecution != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;	/**< The element execution. */
		float* ProgressReceiver;						/**< Pointer to a float value that should receive progress updates. */
	};

	/**
	 * The ElementExecutionUpdateCompositionGraphs struct contains
	 * all necessary information to update the execution composition graphs.
	 */
	struct ElementExecutionUpdateCompositionGraphs
	{
		ElementExecutionUpdateCompositionGraphs(InstaMAT::IElementExecution* const Execution, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat, const float Rotation, const int InShiftWidth, const int InShiftHeight, const bool bInIsSRGBEnabled):
		ElementExecution(Execution),
		ExecutionFormat(ExecutionFormat),
		RotationInRadians(Rotation),
		ShiftWidth(InShiftWidth),
		ShiftHeight(InShiftHeight),
		bIsSRGBEnabled(bInIsSRGBEnabled)
		{
			check(ElementExecution != nullptr);
		}

		InstaMAT::IElementExecution* ElementExecution;			/**< The element execution. */
		InstaMAT::ElementExecutionFormat::Type ExecutionFormat;	/**< The element execution format. */
		float RotationInRadians;								/**< Rotation of the output texture. */
		int ShiftWidth;											/**< Resolution shift for up-/downsampling. */
		int ShiftHeight;										/**< Resolution shift for up-/downsampling. */
		bool bIsSRGBEnabled;									/**< Whether SRGB is enabled. */
	};
}

FInstaMAT::FInstaMAT(InstaMAT::IInstaMAT* const InstaMATAPI) : InstaMAT(InstaMATAPI), InstaMATThread(nullptr)
{
	check(InstaMAT != nullptr);
	InstaMATThread = InstaMAT->AllocTaskThread();

	if (InstaMATThread == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not allocate InstMATThread."));
	}
	else
	{
		InstaMATThread->Start();
	}

	VersionString = InstaMATShared::Version;
}

bool FInstaMAT::Initialize()
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	InstaMATThreadStructs::BackendInitialization Operation(InstaMAT, InstaMAT::IInstaMAT::BackendTypeGPU);

	InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMATThreadStructs::BackendInitialization* const BackendInitialization = static_cast<InstaMATThreadStructs::BackendInitialization*>(UserData);

		check (BackendInitialization->Target != InstaMAT::IInstaMAT::BackendTypeNone);
		
		// Try initialization with target backend, if it fails: fall back to alternative.
		if (!BackendInitialization->InstaMATAPI->Initialize(BackendInitialization->Target, InstaMAT::IInstaMAT::BackendFlagNoPlugins))
		{
			const InstaMAT::IInstaMAT::BackendType AlternativeBackend = BackendInitialization->Target == InstaMAT::IInstaMAT::BackendTypeCPU ? InstaMAT::IInstaMAT::BackendTypeGPU : InstaMAT::IInstaMAT::BackendTypeCPU;
			if (BackendInitialization->InstaMATAPI->Initialize(AlternativeBackend, InstaMAT::IInstaMAT::BackendFlagNoPlugins))
			{
				BackendInitialization->InitializationResult = AlternativeBackend;
			}
		}
		else
		{
			BackendInitialization->InitializationResult = BackendInitialization->Target;
		}

		return true;
	}, &Operation, "InitializeBackend");

	if (Operation.InitializationResult == InstaMAT::IInstaMAT::BackendTypeNone)
	{
		UE_LOG(LogInstaMAT, Fatal, TEXT("Failed to initialize InstaMAT."));
		return false;
	}
	else if (Operation.InitializationResult == InstaMAT::IInstaMAT::BackendTypeCPU)
	{
		UE_LOG(LogInstaMAT, Log, TEXT("InstaMAT is initialized with CPU backend. Execution will be slower."));
	}
	else
	{
		UE_LOG(LogInstaMAT, Log, TEXT("InstaMAT is initialized"));
	}

	return true;
}

bool FInstaMAT::Shutdown()
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	const bool bShutdownSucceeded = InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMAT::IInstaMAT* const InstaMATAPI = static_cast<InstaMAT::IInstaMAT*>(UserData);
		return InstaMATAPI->ShutdownBackend();
	}, InstaMAT, "ShutdownBackend");

	InstaMATThread->Stop();
	InstaMAT->DeallocTaskThread(InstaMATThread);
	InstaMATThread = nullptr;
	InstaMAT->Dealloc();

	return bShutdownSucceeded;
}

bool FInstaMAT::InitializePreviewGenerator()
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	return InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMAT::IInstaMAT* const InstaMATAPI = static_cast<InstaMAT::IInstaMAT*>(UserData);
		return InstaMATAPI->InitializePreviewGenerator();
	}, InstaMAT, "InitializePreviewGenerator");
}

void FInstaMAT::DispatchNotification(const FText& NotificationText, /*SNotificationItem::ECompletionState*/int32 Type)
{
	if (GEditor == nullptr)
		return;
	
	// send a visual notification to the user
	FNotificationInfo Info(NotificationText);
	Info.ExpireDuration = 4.0f;
	Info.bAllowThrottleWhenFrameRateIsLow = true;
	
	const SNotificationItem::ECompletionState State = (SNotificationItem::ECompletionState) Type;
	
	// NOTE: we will create a lambda that will dispatch the notification, else it will disappear immediately due to the huge timeout
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([Info, State]() 
	{
		const TWeakPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
		
		if (Notification.IsValid())
		{
			Notification.Pin()->SetCompletionState(State);
		}
	});
	GEditor->GetTimerManager()->SetTimerForNextTick(TimerDelegate);
}

bool FInstaMAT::AllocPackageFromFile(const FString& FilePath)
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	if (FilePath.IsEmpty() || Packages.Contains(FilePath))
		return true;

	InstaMAT::IGraphPackage* const Package = InstaMAT->AllocPackageFromFile(TCHAR_TO_UTF8(*FilePath), /*persistentResources:*/true);

	if (Package != nullptr)
	{
		Packages.Emplace(FilePath, Package);
	}
	
	return Package != nullptr;
}

InstaMAT::IGraphPackage* FInstaMAT::GetPackageFromFilePath(const FString& FilePath)
{
	check(InstaMAT != nullptr);

	return Packages.Contains(FilePath) ? Packages[FilePath] : nullptr;
}

bool FInstaMAT::GetGraphPreview(const FString& GraphID, const FString& PackageFilePath, uint32& OutWidth, uint32& OutHeight, TArray<FColor>& OutData)
{
	check(InstaMAT != nullptr);
	check(!GraphID.IsEmpty());

	/// The fnCreatePreviewImageFromSampler lambda creates an image from a sampler.
	const auto fnCreatePreviewImageFromSampler = [&](const FString& SaveFileName) -> bool
	{
		if (SaveFileName.IsEmpty())
			return false;

		InstaMATThreadStructs::PreviewSamplerAllocation Operation(this, GraphID, PackageFilePath);
		const bool bDidAllocateSampler = InstaMATThread->AddTaskAndWait([](void* UserData)
		{
			check(UserData != nullptr);
			InstaMATThreadStructs::PreviewSamplerAllocation* const SamplerStruct = static_cast<InstaMATThreadStructs::PreviewSamplerAllocation*>(UserData);
			IInstaMAT* const InstaMATModuleInterface = SamplerStruct->InstaMATModuleInterface;
			const InstaMAT::IGraphObject* GraphObject = InstaMATModuleInterface->GetInstaMAT()->GetGraphObjectByID(TCHAR_TO_UTF8(*SamplerStruct->GraphID));

			if (GraphObject == nullptr && SamplerStruct->PackagePath.IsEmpty())
				return false;
			
			if (GraphObject == nullptr)
			{
				GraphObject = InstaMATModuleInterface->GetGraphObjectFromPackageWithID(SamplerStruct->PackagePath, SamplerStruct->GraphID);

				if (GraphObject == nullptr)
					return false;
			}

			SamplerStruct->OutSampler = InstaMATModuleInterface->GetInstaMAT()->GeneratePreviewForGraphObject(*GraphObject);
			return SamplerStruct->OutSampler != nullptr;
		}, &Operation, "GeneratePreviewForGraphObject");

		if (!bDidAllocateSampler)
			return false;

		Operation.OutSampler->GetSize(&OutWidth, &OutHeight);

		{
			FScopeLock Lock(&Mutex);

			static TArray<FColor> ColorArray;
			ColorArray.SetNumUninitialized(OutWidth * OutHeight);

			static uint32 StaticWidth;
			StaticWidth = OutWidth;

			// the fnSamplerEvaluateLambda converts ColorRGBAF32 to FColor values and saves them in the static array
			InstaMAT::IImageSampler::pfnEvaluateFloatPixelParallel fnSamplerEvaluateLambda = [](const uint32 X, const uint32 Y, const InstaMAT::ColorRGBAF32& InColor, uint32 TaskID)
			{
				FColor Color;
				Color.R = InColor.R * 255.0f;
				Color.G = InColor.G * 255.0f;
				Color.B = InColor.B * 255.0f;
				Color.A = InColor.A * 255.0f;
				ColorArray[X + Y * StaticWidth] = Color;
			};

			Operation.OutSampler->EvaluateFloatParallel(fnSamplerEvaluateLambda);

			{
				OutData.SetNumUninitialized(ColorArray.Num());
				FMemory::Memcpy(OutData.GetData(), ColorArray.GetData(), ColorArray.GetAllocatedSize());
			}

			ColorArray.SetNum(0u);
		}

		// save file to cache
		const FString Path = FPaths::GetPath(SaveFileName);
		if (!SaveFileName.IsEmpty() && FPaths::DirectoryExists(Path) && !FPaths::FileExists(SaveFileName))
		{
			Operation.OutSampler->WritePNG(TCHAR_TO_UTF8(*SaveFileName), 8u, /*allowDithering:*/ true);
		}

		return InstaMAT->DeallocImageSampler(Operation.OutSampler);
	};

	const FString CacheDirectory = InstaMATCacheFilePathUtility::GetDefaultUserPath();
	check(!CacheDirectory.IsEmpty());
	FString UserCacheFileName;

	if (!CacheDirectory.IsEmpty())
	{
		UserCacheFileName = InstaMATCacheFilePathUtility::CreateCachePath(CacheDirectory, GraphID);
	}

	return fnCreatePreviewImageFromSampler(UserCacheFileName);
}

bool FInstaMAT::TryLoadingPreviewImageFromCache(const FString& GraphID, const FString& PackagePath, uint32& OutWidth, uint32& OutHeight, TArray<FColor>& OutData)
{
	if (GraphID.IsEmpty())
		return false;

	const FString EnvironmentDirectory = InstaMATCacheFilePathUtility::GetEnvironmentPath();
	const FString EnvironmentCacheFileName = InstaMATCacheFilePathUtility::CreateCachePath(EnvironmentDirectory, GraphID);

	/// The fnLoadImage lambda retrieves an image from the specified path.
	const auto fnLoadImage = [](const FString& ImagePath, uint32& OutWidth, uint32& OutHeight, TArray<FColor>& OutData) -> bool
	{
		if (!FPaths::FileExists(ImagePath))
			return false;

		TArray64<uint8> FileData;
		if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
			return false;

		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

		const EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
		if (ImageFormat == EImageFormat::Invalid)
			return false;

		TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
		if (!Wrapper.IsValid())
			return false;

		// Decompress the image data
		TArray<uint8> PixelData;
		Wrapper->SetCompressed(FileData.GetData(), FileData.Num());

		if (!Wrapper->GetRaw(ERGBFormat::RGBA, 8, PixelData))
			return false;

		check(PixelData.Num() % 4u == 0u);

		OutWidth = Wrapper->GetWidth();
		OutHeight = Wrapper->GetHeight();
		OutData.Reserve(PixelData.Num() / 4);

		for (int32 Index = 0; Index < PixelData.Num(); Index += 4)
		{
			FColor Color;
			Color.R = PixelData[Index];
			Color.G = PixelData[Index + 1u];
			Color.B = PixelData[Index + 2u];
			Color.A = PixelData[Index + 3u];

			OutData.Push(Color);
		}
		return true;
	};

	if (fnLoadImage(EnvironmentCacheFileName, OutWidth, OutHeight, OutData))
		return true;

	// Get default user path
	const FString UserDirectory = InstaMATCacheFilePathUtility::GetDefaultUserPath();
	FString UserCacheFileName;

	if (UserDirectory.IsEmpty())
		return false;

	UserCacheFileName = InstaMATCacheFilePathUtility::CreateCachePath(UserDirectory, GraphID);

	if (UserCacheFileName.IsEmpty())
		return false;

	if (fnLoadImage(UserCacheFileName, OutWidth, OutHeight, OutData))
		return true;

	return GetGraphPreview(GraphID, PackagePath, OutWidth, OutHeight, OutData);
}

uint32 FInstaMAT::GetInstanceSeedForExecution(const FString& Key)
{
	check(InstaMAT != nullptr);

	if (InstaMAT::IElementExecution* const Execution = GetElementExecutionForKey(Key))
		return Execution->GetInstanceSeed();
	
	return ~0u; 
}

bool FInstaMAT::SetInstanceSeedForExecution(const FString& Key, const uint32& Value)
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	if (InstaMAT::IElementExecution* const Execution = GetElementExecutionForKey(Key))
	{
		const bool bIsSeedSet = Execution->SetInstanceSeed(Value);
		return bIsSeedSet;
	}
	return false;
}

bool FInstaMAT::SetElementImageValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const uint32 Width, const uint32 Height, const InstaMAT::IImageSampler::ComponentType ComponentType, const InstaMAT::IImageSampler::PixelType PixelType, const TArray<uint8>& TextureData, const bool bIsSRGB)
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);
	check(ComponentType == InstaMAT::IImageSampler::ComponentType::ComponentTypeUInt8);

	InstaMAT::IElementExecution* const Execution = GetElementExecutionForKey(ExecutionKey);

	if (Execution == nullptr)
		return false;

	InstaMAT::IGraph* const Graph = Execution->GetInstance();

	if (Graph == nullptr)
		return false;

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(InputIndex, InstaMAT::IGraph::ParameterTypeInput);

	if (Variable == nullptr)
		return false;

	InstaMAT::IImageSampler* const Sampler = InstaMAT->AllocImageSamplerForType(ComponentType, PixelType);
	Sampler->Reallocate(Width, Height);

	uint64 DataSize;
	if (uint8* const WriteTarget = Sampler->GetData(&DataSize))
	{
		// sanity check
		{
			uint32 SamplerWidth;
			uint32 SamplerHeight;
			Sampler->GetSize(&SamplerWidth, &SamplerHeight);
			const uint32 BytesPerPixel = Sampler->GetBytesPerPixel();

			check((SamplerWidth * SamplerHeight * BytesPerPixel) == TextureData.Num());
			check(DataSize == TextureData.Num());
		}

		FMemory::Memcpy(WriteTarget, TextureData.GetData(), DataSize);
	}

	InstaMATThreadStructs::ElementExecutionSetImageSampler Operation(Execution, Variable, Sampler, bIsSRGB);

	const bool bIsSuccessful = InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMATThreadStructs::ElementExecutionSetImageSampler* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionSetImageSampler*>(UserData);
		return Operation->ElementExecution->SetImageSamplerForInputParameter(*Operation->Variable, *Operation->Sampler, Operation->bIsSRGB);
	}, &Operation, "SetImageSamplerForInputParameter");

	InstaMAT->DeallocImageSampler(Sampler);

	return bIsSuccessful;
}

bool FInstaMAT::SetArithmeticValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const InstaMAT::ArithmeticGraphValue& GraphValue, const bool bGraphVariableRequiresReset)
{
	check(InstaMAT != nullptr);

	InstaMAT::IElementExecution* const Execution = GetElementExecutionForKey(ExecutionKey);

	if (Execution == nullptr)
		return false;

	InstaMAT::IGraph* const Graph = Execution->GetInstance();

	if (Graph == nullptr)
		return false;

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(InputIndex, InstaMAT::IGraph::ParameterTypeInput);

	if (Variable == nullptr)
		return false;

	if (bGraphVariableRequiresReset && !Execution->ResetInputParameter(*Variable))
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not reset GraphVariable with name='%s'"), UTF8_TO_TCHAR(Variable->AsObject()->GetName(/*friendlyname:*/true)));
	}

	// NOTE: color values are always interpreted as linear
	return Execution->SetArithmeticValueForInputParameter(*Variable, GraphValue, /*isSRGB:*/ false);
}

bool FInstaMAT::SetElementMeshValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, InstaMAT::IGraphMesh& Mesh)
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	InstaMAT::IElementExecution* const Execution = GetElementExecutionForKey(ExecutionKey);

	if (Execution == nullptr)
		return false;

	InstaMAT::IGraph* const Graph = Execution->GetInstance();

	if (Graph == nullptr)
		return false;

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(InputIndex, InstaMAT::IGraph::ParameterTypeInput);

	if (Variable == nullptr)
		return false;

	check(Variable->GetVariableTypeValue() == InstaMAT::IGraphVariable::TypeElementMesh);

	InstaMATThreadStructs::ElementExecutionSetMesh Operation(Execution, Variable, &Mesh);

	return InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMATThreadStructs::ElementExecutionSetMesh* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionSetMesh*>(UserData);
		return Operation->ElementExecution->SetMeshForInputParameter(*Operation->Variable, *Operation->Mesh);
	}, &Operation, "SetMeshForInputParameter");
}

bool FInstaMAT::SetElementEnumValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const uint32 Value)
{
	check(InstaMAT != nullptr);

	InstaMAT::IElementExecution* const Execution = GetElementExecutionForKey(ExecutionKey);

	if (Execution == nullptr)
		return false;

	InstaMAT::IGraph* const Graph = Execution->GetInstance();

	if (Graph == nullptr)
		return false;

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(InputIndex, InstaMAT::IGraph::ParameterTypeInput);

	if (Variable == nullptr)
		return false;

	return Execution->SetEnumValueForInputParameter(*Variable, Value);
}

bool FInstaMAT::SetElementStringValueForInputParameter(const FString& ExecutionKey, const uint32 InputIndex, const FString& Value)
{
	check(InstaMAT != nullptr);

	InstaMAT::IElementExecution* const Execution = GetElementExecutionForKey(ExecutionKey);

	if (Execution == nullptr)
		return false;

	InstaMAT::IGraph* const Graph = Execution->GetInstance();

	if (Graph == nullptr)
		return false;

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(InputIndex, InstaMAT::IGraph::ParameterTypeInput);

	if (Variable == nullptr)
		return false;

	return Execution->SetStringValueForInputParameter(*Variable, TCHAR_TO_UTF8(*Value));
}

bool FInstaMAT::DeallocPackage(const FString& FilePath)
{
	check(InstaMAT != nullptr);

	InstaMAT::IGraphPackage* const Package = GetPackageFromFilePath(FilePath);

	if (Package == nullptr)
		return true;

	Packages.Remove(FilePath);
	return InstaMAT->DeallocPackage(Package);
}

TTuple<EInstaMATTextureSize, EInstaMATTextureSize> FInstaMAT::GetDefaultPreviewResolutionSettings() const
{
	const UInstaMATSettings* const DefaultObject = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
	check(DefaultObject != nullptr);

	return TTuple<EInstaMATTextureSize, EInstaMATTextureSize>(DefaultObject->PreviewResolutionWidth, DefaultObject->PreviewResolutionHeight);
}

TTuple<EInstaMATTextureSize, EInstaMATTextureSize> FInstaMAT::GetDefaultResolutionSettings() const
{
	const UInstaMATSettings* const DefaultObject = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
	check(DefaultObject != nullptr);

	return TTuple<EInstaMATTextureSize, EInstaMATTextureSize>(DefaultObject->ResolutionWidth, DefaultObject->ResolutionHeight);
}

EInstaMATExecutionFormat FInstaMAT::GetDefaultExecutionFormatSettings() const
{
	const UInstaMATSettings* const DefaultObject = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
	check(DefaultObject != nullptr);

	return DefaultObject->ExecutionFormat;
}

bool FInstaMAT::DeallocPackage(InstaMAT::IGraphPackage* const Package)
{
	check(InstaMAT != nullptr);

	if (Package == nullptr)
		return true;

	// check if the package has an map entry and delete from map if necessary
	for (const auto& [Key, Value] : Packages)
	{
		if (Value == Package)
		{
			Packages.Remove(Key);
			break;
		}
	}

	return InstaMAT->DeallocPackage(Package);
}

bool FInstaMAT::GetGraphObjectsInPackage(const InstaMAT::IGraphPackage& Package, InstaMAT::IGraphObject*** OutGraphObjects)
{
	check(InstaMAT != nullptr);
	check(OutGraphObjects != nullptr);

	return InstaMAT->GetGraphObjectsInPackage(Package, OutGraphObjects);
}

uint32 FInstaMAT::RegisterExternalAssetsFolder(const FString& Path)
{
	if (Path.IsEmpty())
		return 0u;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Ensure that the directory exists
	if (!PlatformFile.DirectoryExists(*Path))
	{
		if (!PlatformFile.CreateDirectory(*Path))
		{
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Failed to register directory as external asset directory. (Reason: Assets directory does not exist and failed to be created for path='%s')."), *Path);
			return 0u;
		}
	}

	check(InstaMAT != nullptr);

	UE_LOG(LogInstaMAT, Log, TEXT("InstaMAT: Registering external asset path='%s'"), *Path);
	return InstaMAT->RegisterExternalAssetFolder(TCHAR_TO_UTF8(*Path));
}

void FInstaMAT::UnregisterAllExternalAssetsFolder()
{
	check(InstaMAT != nullptr);

	UE_LOG(LogInstaMAT, Log, TEXT("InstaMAT: Unregistering all external asset paths."));
	InstaMAT->UnregisterExternalAssetFolders();
}

const InstaMAT::IGraphObject* FInstaMAT::GetGraphObjectFromPackageWithID(const FString& FilePath, const FString& GraphID)
{
	check(InstaMAT != nullptr);

	if (FilePath.IsEmpty())
		return GetGraphObjectWithID(GraphID);

	InstaMAT::IGraphPackage* const Package = GetPackageFromFilePath(FilePath);

	if (Package == nullptr)
		return nullptr;

	InstaMAT::IGraphObject** GraphObjects;
	if (!GetGraphObjectsInPackage(*Package, &GraphObjects))
		return nullptr;

	const uint32 kBufferSize = 256u;
	ANSICHAR Temp[kBufferSize];

	for (auto Iterator = GraphObjects; *Iterator != nullptr; Iterator++)
	{
		InstaMAT::IGraphObject* const GraphObject = *Iterator;
		GraphObject->GetID(Temp, kBufferSize);

		if (GraphID == FString(UTF8_TO_TCHAR(Temp)))
			return GraphObject;
	}
	return nullptr;
}

const InstaMAT::IGraphObject* FInstaMAT::GetGraphObjectWithID(const FString& GraphID)
{
	check(InstaMAT != nullptr);

	if (GraphID.IsEmpty())
		return nullptr;

	return InstaMAT->GetGraphObjectByID(TCHAR_TO_UTF8(*GraphID));
}

InstaMAT::IImageSampler* FInstaMAT::GetImageSamplerForExecutionAndGraphVariable(InstaMAT::IElementExecution* const Execution, const InstaMAT::IGraphVariable* const Variable)
{
	if (Execution == nullptr || Variable == nullptr)
		return nullptr;

	check(InstaMATThread != nullptr);

	InstaMATThreadStructs::ElementExecutionAllocImageSampler Operation(Execution, Variable);

	const bool bSucceeded = InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMATThreadStructs::ElementExecutionAllocImageSampler* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionAllocImageSampler*>(UserData);
		InstaMAT::IGraphVariable* const CompositionGraph = Operation->ElementExecution->GetCompositionGraphOutputForOutputParameter(*Operation->Variable);

		if (CompositionGraph == nullptr)
			return false;
			
		Operation->OutSampler = Operation->ElementExecution->AllocImageSamplerForOutputParameter(*CompositionGraph, /*srgb:*/ false);
		return Operation->OutSampler != nullptr;
	}, &Operation, "AllocImageSamplerForOutputParameter");

	return Operation.OutSampler;
}

void FInstaMAT::ClearUnusedElementExecutionsAndFreeVideoMemory()
{
	check(InstaMAT != nullptr);

	ON_SCOPE_EXIT
	{
		InstaMATThread->AddTaskAndWait([](void* UserData)
		{
			check(UserData != nullptr);

			InstaMAT::IInstaMAT* const InstaMATAPI = static_cast<InstaMAT::IInstaMAT*>(UserData);
			InstaMATAPI->ReleaseVideoMemory();
			return true;
		}, InstaMAT, "ReleaseVideoMemory");
	};

	if (ElementExecutions.Num() == 0)
		return;

	for (auto& [_, Execution] : ElementExecutions)
	{
		if (Execution != nullptr)
		{
			DeallocElementExecutionOnTaskThread(Execution);
		}
	}
	ElementExecutions.Empty(); 
}

bool FInstaMAT::IsElementExecutionAllocatedForKey(const FString& Key) const
{
	check(InstaMAT != nullptr);
	return ElementExecutions.Contains(Key);
}

bool FInstaMAT::AllocElementExecutionFromTemplate(const FString& Key, const InstaMAT::IGraphTemplate& Template, const InstaMAT::ElementExecutionFlags::Type ExecutionFlag)
{
	check(InstaMAT != nullptr);

	if (ElementExecutions.Contains(Key))
		return true;

	// make sure no other element executions remove from memory
	ClearUnusedElementExecutionsAndFreeVideoMemory();

	InstaMAT::IElementExecution* const ElementExecution = InstaMAT->AllocElementExecution();

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not execute ElementExecution for Template with name ='%s'"), UTF8_TO_TCHAR(Template.AsObject()->GetName(/*friendlyname:*/true)));
		return false;
	}

	if (!ElementExecution->CreateInstance(Template, ExecutionFlag))
	{
		DeallocElementExecutionOnTaskThread(ElementExecution);
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not create Instance for ElementExecution for Template with name ='%s'.\n"), UTF8_TO_TCHAR(Template.AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	ElementExecutions.Emplace(Key, ElementExecution);
	return true;
}

bool FInstaMAT::AllocElementExecution(const FString& Key, const FString& GraphID, const InstaMAT::IGraph* Graph, const InstaMAT::ElementExecutionFlags::Type ExecutionFlag)
{
	check(InstaMAT != nullptr);

	if (ElementExecutions.Contains(Key))
		return true;

	// make sure no other element executions are alive, remove from memory
	ClearUnusedElementExecutionsAndFreeVideoMemory();

	if (Graph == nullptr)
	{
		if (const InstaMAT::IGraphObject* const GraphObject = InstaMAT->GetGraphObjectByID(TCHAR_TO_UTF8(*GraphID)))
		{
			Graph = GraphObject->AsGraph();
		}
	}

	if (Graph == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve graph for graph ID='%s'."), *GraphID);
		return false;
	}

	InstaMAT::IElementExecution* const ElementExecution = InstaMAT->AllocElementExecution();

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not execute ElementExecution for Graph with name='%s'"), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyname:*/true)));
		return false;
	}

	if (!ElementExecution->CreateInstance(*Graph, ExecutionFlag))
	{
		DeallocElementExecutionOnTaskThread(ElementExecution);
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not create Instance for ElementExecution for Graph with name='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	ElementExecutions.Emplace(Key, ElementExecution);
	return true;
}

bool FInstaMAT::DeallocElementExecution(const FString& Key)
{
	check(InstaMAT != nullptr);

	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
		return true;

	ElementExecutions.Remove(Key);

	const bool bDeallocated = DeallocElementExecutionOnTaskThread(ElementExecution);
	if (!bDeallocated)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not deallocate ElementExecution for key='%s'."), *Key);
	}

	return bDeallocated;
}

bool FInstaMAT::DeallocElementExecutionOnTaskThread(InstaMAT::IElementExecution* const Execution)
{
	check(InstaMATThread != nullptr);
	check(InstaMAT != nullptr);
	check(Execution != nullptr);
	
	InstaMATThreadStructs::ElementExecutionDealloc Operation(Execution, InstaMAT);

	return InstaMATThread->AddTaskAndWait([](void* UserData) -> bool {
		check(UserData != nullptr);

		InstaMATThreadStructs::ElementExecutionDealloc* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionDealloc*>(UserData);
		return Operation->InstaMATAPI->DeallocElementExecution(Operation->ElementExecution);
	}, &Operation, "ElementExecutionDealloc");
}

InstaMAT::IElementExecution* FInstaMAT::GetElementExecutionForKey(const FString& Key)
{
	check(InstaMAT != nullptr);
	return ElementExecutions.Contains(Key) ? ElementExecutions[Key] : nullptr;
}

bool FInstaMAT::SetElementExecutionFormat(const FString& Key, const uint32 Width, const uint32 Height, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat)
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve ElementExecution for key='%s'."), *Key);
		return false;
	}

	InstaMATThreadStructs::ElementExecutionSetFormat Operation(ElementExecution, Width, Height, ExecutionFormat);

	return InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMATThreadStructs::ElementExecutionSetFormat* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionSetFormat*>(UserData);
		return Operation->ElementExecution->SetFormat(Operation->Width, Operation->Height, Operation->Format > InstaMAT::ElementExecutionFormat::Normalized16 ? Operation->Format : InstaMAT::ElementExecutionFormat::Normalized16);
	}, &Operation, "ElementExecutionSetFormat");
}

bool FInstaMAT::SetCompositionGraphSettings(const FString& Key, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat, const float RotationInRadians, const int32 ShiftWidth, const int32 ShiftHeight)
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve ElementExecution for Key='%s'."), *Key);
		return false;
	}

	InstaMATThreadStructs::ElementExecutionUpdateCompositionGraphs Operation(ElementExecution, ExecutionFormat, RotationInRadians, ShiftWidth, ShiftHeight, ExecutionFormat == InstaMAT::ElementExecutionFormat::Normalized8);

	const bool bSucceeded = InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);

		InstaMATThreadStructs::ElementExecutionUpdateCompositionGraphs* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionUpdateCompositionGraphs*>(UserData);
		const InstaMAT::IGraph* const Graph = Operation->ElementExecution->GetInstance();

		// iterate the output parameters of our instance
		const uint32 ParameterCount = Graph->GetParameterCount(InstaMAT::IGraph::ParameterTypeOutput);

		for (uint32 ParameterIndex = 0u; ParameterIndex < ParameterCount; ParameterIndex++)
		{
			const InstaMAT::IGraphVariable* const Variable = Operation->ElementExecution->GetInstance()->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeOutput);

			if (Variable == nullptr)
				continue;

			if (InstaMAT::IGraph* const CompositionGraph = Operation->ElementExecution->GetCompositionGraphForOutputParameter(*Variable))
			{
				// NOTE: check if output is normal map and fix the output greenchannel
				if (const InstaMAT::IGraphObject* const Object = Variable->AsObject())
				{
					const FString Name = FString(Object->GetName(/*friendlyName:*/false));

					if (Name.ToLower().Contains(TEXT("normal")))
					{
						if (InstaMAT::IGraphVariable* const GreenOutVariable = CompositionGraph->GetParameterWithName("GreenOut", InstaMAT::IGraph::ParameterTypeInput))
						{
							InstaMAT::ArithmeticGraphValue Value;
							Value.Vector2FValue[0] = 1.0f;
							Value.Vector2FValue[1] = 0.0f;
							Operation->ElementExecution->SetArithmeticValueForInputParameter(*GreenOutVariable, Value, /*isSRGB:*/ false);
						}
					}
					
					if (InstaMAT::IGraphVariable* const SRGB = CompositionGraph->GetParameterWithName("SRGB", InstaMAT::IGraph::ParameterTypeInput))
					{
						// NOTE: UE only supports SRGB for 8-bit textures. If bIsSRGBEnabled is false, we should disable SRGB even if the variable outputs in that color space.
						InstaMAT::ArithmeticGraphValue Value;
						Value.BooleanValue = Operation->bIsSRGBEnabled && Variable->GetColorSpaceTypeValue() == InstaMAT::IGraphVariable::ColorSpaceTypeSRGB;
						Operation->ElementExecution->SetArithmeticValueForInputParameter(*SRGB, Value, /*isSRGB:*/ Value.BooleanValue);
					}

					if (InstaMAT::IGraphVariable* const Rotation = CompositionGraph->GetParameterWithName("Rotate", InstaMAT::IGraph::ParameterTypeInput))
					{
						InstaMAT::ArithmeticGraphValue Value;
						Value.Float32Value = Operation->RotationInRadians;
						Operation->ElementExecution->SetArithmeticValueForInputParameter(*Rotation, Value, /*isSRGB:*/ false);
					}
				}
			}

			Operation->ElementExecution->SetCompositionGraphFormatForOutputParameter(*Variable, Operation->ShiftWidth, Operation->ShiftHeight, Operation->ExecutionFormat);
		}

		return Operation->ElementExecution->Execute([](const InstaMAT::IGraph& Graph, const float Progress) { return true; });
	}, &Operation, "SetCompositionGraph");

	return bSucceeded;
}

bool FInstaMAT::ExecuteElementExecutionForKey(const FString& Key, bool bIsHighColorDepth, const InstaMAT::ElementExecutionFormat::Type ExecutionFormat, const float RotationInRadians, const int32 ShiftWidth, const int32 ShiftHeight, float* ProgressReceiver)
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve ElementExecution for Key ='%s'.\n"), *Key);
		return false;
	}

	InstaMATThreadStructs::ElementExecutionExecute Operation(ElementExecution, ProgressReceiver);

	const bool bSucceeded = InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);

		InstaMATThreadStructs::ElementExecutionExecute* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionExecute*>(UserData);
		float* const ProgressReceiver = Operation->ProgressReceiver;

		static float* FloatProgressReceiver = nullptr;

		FloatProgressReceiver = ProgressReceiver;
		if (ProgressReceiver != nullptr)
		{
			*ProgressReceiver = 0.0f;
		}

		/// The fnProgressCallback lambda callback to report the progress.
		const auto fnProgressCallback = [](const InstaMAT::IGraph& Graph, const float Progress) -> bool
		{
			if (FloatProgressReceiver != nullptr)
			{
				*FloatProgressReceiver = Progress;
			}

			return true;
		};
		
		const bool bResult = Operation->ElementExecution->Execute(fnProgressCallback);

		FloatProgressReceiver = nullptr;

		return bResult;
	}, &Operation, "Execute");

	return bSucceeded;
}

InstaMAT::IImageSampler* FInstaMAT::GetImageSamplerForOutputParameter(const FString& Key, const uint32 OutputParameter)
{
	check(InstaMAT != nullptr);
	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve ElementExecution for key='%s'."), *Key);
		return nullptr;
	}

	InstaMAT::IGraph* const Graph = ElementExecution->GetInstance();

	if (Graph == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve Graph Instance for key='%s'."), *Key);
		return nullptr;
	}

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(OutputParameter, InstaMAT::IGraph::ParameterTypeOutput);

	if (Variable == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve Output Variable for Graph with name='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return nullptr;
	}

	InstaMAT::IImageSampler* const Sampler = GetImageSamplerForExecutionAndGraphVariable(ElementExecution, Variable);

	if (Sampler == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not create Image Sampler for Graph with name='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return nullptr;
	}

	return Sampler;
}

bool FInstaMAT::GetDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, TArray<uint8>& OutData)
{
	check(InstaMAT != nullptr);
	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve ElementExecution for key='%s'."), *Key);
		return false;
	}

	InstaMAT::IGraph* const Graph = ElementExecution->GetInstance();

	if (Graph == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve Graph Instance for key='%s'."), *Key);
		return false;
	}

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeOutput);

	if (Variable == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve Output Variable for Graph with name='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	InstaMAT::IImageSampler* const Sampler = GetImageSamplerForExecutionAndGraphVariable(ElementExecution, Variable);

	if (Sampler == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not create Image Sampler for Graph with name='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	uint64 DataSize = 0u;
	if (uint8* const Data = Sampler->GetData(&DataSize))
	{
		OutData.SetNumUninitialized(DataSize);
		FMemory::Memcpy(OutData.GetData(), Data, DataSize);
	}

	InstaMAT->DeallocImageSampler(Sampler);
	return true;
}

bool FInstaMAT::GetColorSpaceForOutputParameter(const FString& Key, const uint32 ParameterIndex, InstaMAT::IGraphVariable::ColorSpaceType& ColorSpace)
{
	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve ElementExecution for key='%s'."), *Key);
		return false;
	}

	InstaMAT::IGraph* const Graph = ElementExecution->GetInstance();

	if (Graph == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve Graph Instance for key='%s'."), *Key);
		return false;
	}

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeOutput); 

	if (Variable == nullptr)
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve Output Variable for Graph with name='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}
	
	if (!InstaMATGraphVariableUtility::IsImageType(Variable->GetVariableTypeValue()))
	{
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Output Variable type is not an image='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	ColorSpace = Variable->GetColorSpaceTypeValue();
	return true;
}

bool FInstaMAT::GetMeshDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, const InstaMAT::IGraphMesh** OutMesh)
{
	check(OutMesh != nullptr);
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not retrieve ElementExecution for key='%s'."), *Key);
		return false;
	}

	InstaMAT::IGraph* const Graph = ElementExecution->GetInstance();

	if (Graph == nullptr)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not retrieve Graph Instance for key='%s'."), *Key);
		return false;
	}

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeOutput);

	if (Variable == nullptr)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not retrieve Output Variable for Graph with name='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	if (Variable->GetVariableTypeValue() != InstaMAT::IGraphVariable::TypeElementMesh)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Output Variable type is not mesh='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	InstaMATThreadStructs::ElementExecutionGetMesh Operation(ElementExecution, Variable);

	InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMATThreadStructs::ElementExecutionGetMesh* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionGetMesh*>(UserData);
		Operation->OutMesh = Operation->ElementExecution->GetMeshForOutputParameter(*Operation->Variable);

		return true;
	}, &Operation, "GetMeshForOutputParameter");

	*OutMesh = Operation.OutMesh;
	return OutMesh != nullptr;
};

bool FInstaMAT::GetSceneDataForOutputParameter(const FString& Key, const uint32 ParameterIndex, const InstaMAT::IGraphScene** OutScene)
{
	check(OutScene != nullptr);
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	InstaMAT::IElementExecution* const ElementExecution = GetElementExecutionForKey(Key);

	if (ElementExecution == nullptr)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not retrieve ElementExecution for key='%s'."), *Key);
		return false;
	}

	InstaMAT::IGraph* const Graph = ElementExecution->GetInstance();

	if (Graph == nullptr)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not retrieve Graph Instance for key='%s'."), *Key);
		return false;
	}

	InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeOutput);

	if (Variable == nullptr)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Could not retrieve Output Variable for Graph with name='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	if (Variable->GetVariableTypeValue() != InstaMAT::IGraphVariable::TypeElementScene)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Output Variable type is not scene='%s'."), UTF8_TO_TCHAR(Graph->AsObject()->GetName(/*friendlyName:*/true)));
		return false;
	}

	InstaMATThreadStructs::ElementExecutionGetScene Operation(ElementExecution, Variable);

	InstaMATThread->AddTaskAndWait([](void* UserData)
	{
		check(UserData != nullptr);
		InstaMATThreadStructs::ElementExecutionGetScene* const Operation = static_cast<InstaMATThreadStructs::ElementExecutionGetScene*>(UserData);
		Operation->OutScene = Operation->ElementExecution->GetSceneForOutputParameter(*Operation->Variable);

		return true;
	}, &Operation, "GetSceneForOutputParameter");

	*OutScene = Operation.OutScene;
	return OutScene != nullptr;
}

bool FInstaMAT::ConvertGraphMeshToMeshDescription(const InstaMAT::IGraphMesh& Mesh, FMeshDescription& MeshDescription)
{
	InstaMATMeshUtility::GraphMeshToMeshDescription(Mesh, MeshDescription);
	return true;
}

bool FInstaMAT::ConvertMeshDescriptionToGraphMesh(const FMeshDescription& MeshDescription, InstaMATMesh& TargetMesh)
{
	InstaMATMeshUtility::MeshDescriptionToGraphMesh(MeshDescription, TargetMesh);
	return true;
}

void FInstaMAT::GetInputAndOutputParameterDefinitions(const FString& GraphID, TArray<FInstaMATGraphObjectInputData>& OutInputArray, TArray<FInstaMATGraphObjectOutputData>& OutOutputArray)
{
	OutInputArray.Empty();
	OutOutputArray.Empty();

	/// The fnGetGraphInputDefinitions lambda gets the input definitions for a graph.
	const auto fnGetGraphInputDefinitions = [&OutInputArray](const InstaMAT::IGraph* const Graph)
	{
		const uint32 ParameterCount = Graph->GetParameterCount(InstaMAT::IGraph::ParameterTypeInput);

		for (uint32 ParameterIndex = 0u; ParameterIndex < ParameterCount; ParameterIndex++)
		{
			const InstaMAT::IGraphVariable* const InputGraphVariable = Graph->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeInput);
			const InstaMAT::IGraphVariable::Type VariableType = InputGraphVariable->GetVariableTypeValue();

			if (VariableType == InstaMAT::IGraphVariable::Type::TypeElementResource ||
				VariableType == InstaMAT::IGraphVariable::Type::TypeElementPointCloud ||
				VariableType == InstaMAT::IGraphVariable::Type::TypeElementScene)
				continue;

			OutInputArray.Add(FInstaMATGraphObjectInputData());
			FInstaMATGraphObjectInputData& Input = OutInputArray.Last();
			const InstaMAT::IGraphObject* const VariableGraphObject = InputGraphVariable->AsObject();
			FString InputCategory = FString(UTF8_TO_TCHAR(VariableGraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyCategory)));

			Input.Name = FString(UTF8_TO_TCHAR(VariableGraphObject->GetName(/*friendlyName:*/ true)));
			Input.TypeString = FString(UTF8_TO_TCHAR(InputGraphVariable->GetVariableTypeString()));
			Input.Documentation = FString(UTF8_TO_TCHAR(VariableGraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyDocumentation)));

			if (Input.Documentation.Len() > 0)
			{
				UE_LOG(LogInstaMAT, Warning, TEXT("Input: %s Documentation: %s"), *Input.Name, *Input.Documentation);
			}

			Input.Category = InputCategory.IsEmpty() ? FString(TEXT("Input")) : InputCategory;
			Input.Type = VariableType;

			if (VariableGraphObject->ContainsMetaDataKeyChar(InstaMAT::MetaData::KeyUIControlType))
			{
				Input.ControlType = (InstaMAT::IGraphVariable::UIControlType) (VariableGraphObject->GetMetaDataAsCharInt32(InstaMAT::MetaData::KeyUIControlType));
			}
			else
			{
				Input.ControlType = InstaMAT::IGraphVariable::UIControlTypeSpinBox;
			}

			if (VariableType == InstaMAT::IGraphVariable::Type::TypeEnumValue)
			{
				const uint32 EnumValue = InputGraphVariable->GetEnumValueUI32();
				Input.StringDefaultValue = FString(UTF8_TO_TCHAR(InputGraphVariable->GetEnumValueString(EnumValue)));
			}
			else if (VariableType == InstaMAT::IGraphVariable::Type::TypeElementString)
			{
				Input.StringDefaultValue = FString(UTF8_TO_TCHAR(InputGraphVariable->GetElementStringValue()));
			}
			else
			{
				Input.DefaultValue = InputGraphVariable->GetArithmeticValue();
			}
		}
	};

	/// The fnGetGraphOutputDefinitions lambda gets the output definitions for a graph.
	const auto fnGetGraphOutputDefinitions = [&OutOutputArray](const InstaMAT::IGraph* const Graph)
	{
		const uint32 OutputParameterCount = Graph->GetParameterCount(InstaMAT::IGraph::ParameterTypeOutput);

		for (uint32 ParameterIndex = 0u; ParameterIndex < OutputParameterCount; ParameterIndex++)
		{
			const InstaMAT::IGraphVariable* const OutputGraphVariable = Graph->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeOutput);
			const InstaMAT::IGraphVariable::Type VariableType = OutputGraphVariable->GetVariableTypeValue();

			if (const InstaMAT::IGraphObject* const VariableGraphObject = OutputGraphVariable->AsObject())
			{
				OutOutputArray.Add(FInstaMATGraphObjectOutputData());
				FInstaMATGraphObjectOutputData& Output = OutOutputArray.Last();
				Output.Type = VariableType;
				Output.Name = FString(UTF8_TO_TCHAR(VariableGraphObject->GetName(/*friendlyName:*/ true)));
				Output.Documentation = FString(UTF8_TO_TCHAR(VariableGraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyDocumentation)));
				Output.TypeString = FString(UTF8_TO_TCHAR(OutputGraphVariable->GetVariableTypeString()));

				if (VariableType == InstaMAT::IGraphVariable::Type::TypeAtomOutputImage ||
					VariableType == InstaMAT::IGraphVariable::Type::TypeAtomOutputImageGray ||
					VariableType == InstaMAT::IGraphVariable::Type::TypeElementImage ||
					VariableType == InstaMAT::IGraphVariable::Type::TypeElementImageGray)
				{
					Output.ColorSpace = OutputGraphVariable->GetColorSpaceTypeValue();
				}
			}
		}
	};

	if (const InstaMAT::IGraphObject* const GraphObject = InstaMAT->GetGraphObjectByID(TCHAR_TO_UTF8(*GraphID)))
	{
		if (const InstaMAT::IGraphTemplate* const TemplateGraph = GraphObject->AsTemplate())
		{
			InstaMAT::IElementExecution* const Execution = InstaMAT->AllocElementExecution();
			check(Execution != nullptr);

			if (Execution != nullptr)
			{
				if (!Execution->CreateInstance(*TemplateGraph, InstaMAT::ElementExecutionFlags::None))
				{
					UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Failed to retrieve graph instance for template graph."));
					InstaMAT->DeallocElementExecution(Execution);
				}

				const InstaMAT::IGraph* const Graph = Execution->GetInstance();

				fnGetGraphInputDefinitions(Graph);
				fnGetGraphOutputDefinitions(Graph);

				InstaMAT->DeallocElementExecution(Execution);
			}
		}
		else if (const InstaMAT::IGraph* const Graph = GraphObject->AsGraph())
		{
			fnGetGraphInputDefinitions(Graph);
			fnGetGraphOutputDefinitions(Graph);
		}
	}
}

bool FInstaMAT::IsPBRMaterialGraph(const InstaMAT::IGraphObject* const GraphObject)
{
	check(InstaMAT != nullptr);
	check(GraphObject != nullptr);
	
	const InstaMAT::IGraph* Graph = GraphObject->AsGraph();

	if (Graph == nullptr)
	{
		// check if template
		if (const InstaMAT::IGraphTemplate* const GraphTemplate = GraphObject->AsTemplate())
		{
			ANSICHAR Buffer[256u];
			if (GraphTemplate->GetClassID(Buffer, sizeof(Buffer)) > 0u)
			{
				if (const InstaMAT::IGraphObject* const Object = InstaMAT->GetGraphObjectByID(Buffer))
				{
					Graph = Object->AsGraph();
				}
			}
		}

		if (Graph == nullptr)
			return false;
	}
	
	static const TArray<FString> DiffuseOutputNames = { TEXT("Diffuse"), TEXT("BaseColor"), TEXT("Base Color"), TEXT("Albedo"), TEXT("Base_Color")};
	const uint32 OutputParameterCount = Graph->GetParameterCount(InstaMAT::IGraph::ParameterTypeOutput);

	for (uint32 Index = 0u; Index < OutputParameterCount; Index++)
	{
		const InstaMAT::IGraphVariable* const Variable = Graph->GetParameterAtIndex(Index, InstaMAT::IGraph::ParameterTypeOutput);
		
		if (Variable == nullptr)
			continue;

		const FString VariableName = FString(UTF8_TO_TCHAR(Variable->AsObject()->GetName(/*friendlyName:*/ true)));

		if (DiffuseOutputNames.FindByPredicate([VariableName](const FString& Value) { return Value.Compare(VariableName, ESearchCase::IgnoreCase) == 0; }) != nullptr)
			return true;
	}

	return false;
}

bool FInstaMAT::AllocPackageFromPath(const FString& FilePath)
{
	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);

	const bool bIsPackageLoaded = InstaMAT->AllocPackageFromFile(TCHAR_TO_UTF8(*FilePath), /*persistentResource:*/ true) != nullptr;
	return bIsPackageLoaded;
}

uint32 FInstaMAT::LoadEnvironmentPackageFromPath(const FString& Path, const bool bIsSystemLibrary)
{
	if (Path.IsEmpty())
		return false;

	check(InstaMAT != nullptr);
	check(InstaMATThread != nullptr);
	
	UE_LOG(LogInstaMAT, Log, TEXT("InstaMAT: Loading packages from path='%s'"), *Path);

	const uint32 LoadedPackages = InstaMAT->LoadPackagesAtPath(TCHAR_TO_UTF8(*Path), /*persistentResource:*/ true, bIsSystemLibrary);
	return LoadedPackages;
}

const TArray<FString>& FInstaMAT::GetUserCategories(const bool bIsUpdateEnforced)
{
	check(InstaMAT != nullptr);
	static TArray<FString> CategoryValues;

	FScopeLock Lock(&Mutex);

	if (CategoryValues.Num() == 0 || bIsUpdateEnforced)
	{
		CategoryValues.Reset();

		const ANSICHAR** Categories = nullptr;

		if (!InstaMAT->GetCategories(&Categories))
			return CategoryValues;

		for (auto CategoryIterator = Categories; *CategoryIterator != nullptr; CategoryIterator++)
		{
			const ANSICHAR* const Category = *CategoryIterator;

			if (Category == nullptr)
				continue;

			const FString CategoryString(UTF8_TO_TCHAR(Category));

			// check if category holds any objects
			InstaMAT::IGraphObject** GraphObjects;
			InstaMAT->GetGraphObjectsInCategory(Category, &GraphObjects);
			bool bContainsValidGraphs = false;

			for (auto GraphObjectIterator = GraphObjects; *GraphObjectIterator != nullptr; GraphObjectIterator++)
			{
				const InstaMAT::IGraphObject* const GraphObject = *GraphObjectIterator;

				if (GraphObject == nullptr)
					continue;

				if (const InstaMAT::IGraphPackage* const Package = GraphObject->GetParentPackage())
				{
					const FString PackageType = UTF8_TO_TCHAR(Package->GetOriginType());

					if (PackageType != kInstaMATUserPackageType)
						continue;
				}
				else
				{
					// built-in package
					continue;
				}

				if (!InstaMATGraphUtility::IsGraphObjectValid(GraphObject))
					continue;

				bContainsValidGraphs = true;
				break;
			}

			if (!bContainsValidGraphs)
				continue;

			CategoryValues.AddUnique(CategoryString);
		}

		CategoryValues.Sort();
	}

	return CategoryValues;
}

const TArray<FString>& FInstaMAT::GetCategories(const bool bIsUpdateEnforced)
{
	check(InstaMAT != nullptr);
	static TArray<FString> CategoryValues;

	FScopeLock Lock(&Mutex);

	if (CategoryValues.Num() == 0 || bIsUpdateEnforced)
	{
		CategoryValues.Reset();

		const ANSICHAR** Categories = nullptr;

		if (!InstaMAT->GetCategories(&Categories))
			return CategoryValues;

		for (auto CategoryIterator = Categories; *CategoryIterator != nullptr; CategoryIterator++)
		{
			const ANSICHAR* const Category = *CategoryIterator;

			if (Category == nullptr)
				continue;

			const FString CategoryString(UTF8_TO_TCHAR(Category));
			if (CategoryString.Contains("InstaLOD"))
				continue;

			// check if category holds any objects
			InstaMAT::IGraphObject** GraphObjects;
			InstaMAT->GetGraphObjectsInCategory(Category, &GraphObjects);
			bool bContainsValidGraphs = false;

			for (auto GraphObjectIterator = GraphObjects; *GraphObjectIterator != nullptr; GraphObjectIterator++)
			{ 
				const InstaMAT::IGraphObject* const GraphObject = *GraphObjectIterator;
				if (GraphObject == nullptr)
					continue;

				if (!InstaMATGraphUtility::IsGraphObjectValid(GraphObject))
					continue;

				bContainsValidGraphs = true;
				break;
			}

			if (!bContainsValidGraphs)
				continue;

			CategoryValues.AddUnique(CategoryString);
		}

		CategoryValues.Sort();
	}

	Lock.Unlock();
	return CategoryValues;
}

FString FInstaMAT::GetMachineKeyAsFString()
{
	check(InstaMAT != nullptr);

	static FString MachineKey;

	if (!MachineKey.IsEmpty())
		return MachineKey;

	constexpr uint32 kBufferSize = 256u;
	ANSICHAR Buffer[kBufferSize];
	uint64 StringSize;

	const uint64 WrittenSize = InstaMAT->GetMachineAuthorizationKey(Buffer, sizeof(Buffer), &StringSize);
	MachineKey = FString(UTF8_TO_TCHAR(Buffer));

	return MachineKey;
}

bool FInstaMAT::IngestLicense(const FString& FilePath)
{
	check(InstaMAT != nullptr);

	if (FilePath.IsEmpty())
		return false;

	return InstaMAT->IngestMachineAuthorizationLicense(TCHAR_TO_UTF8(*FilePath));
}

bool FInstaMAT::IsCachedPreviewImageAvailable(const FString& GraphID)
{
	if (GraphID.IsEmpty())
		return false;

	// Get default user path
	UInstaMATSettings* const UserSettings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
	UserSettings->LoadConfig();
	UserSettings->EnsureDefaultUserPathIsSet();
	UserSettings->SaveConfig();

#if PLATFORM_WINDOWS
	const FString EnvironmentDirectory = UserSettings->EnvironmentFolder;
	FString EnvironmentCacheFileName = FPaths::Combine(EnvironmentDirectory, TEXT("Cache"), GraphID + TEXT(".png"));
#elif PLATFORM_MAC
	const FString EnvironmentDirectory = UserSettings->EnvironmentFolder;
	FString EnvironmentCacheFileName = FPaths::Combine(EnvironmentDirectory, TEXT("Contents/Resources/Cache"), GraphID + TEXT(".png"));
#endif
	FPaths::NormalizeFilename(EnvironmentCacheFileName);
	EnvironmentCacheFileName = FPaths::ConvertRelativePathToFull(EnvironmentCacheFileName);
	
	if (FPaths::FileExists(EnvironmentCacheFileName))
		return true;

	FInstaMATUserDirectory* const DefaultUserDirectory = UserSettings->UserFolders.FindByPredicate([](const FInstaMATUserDirectory& Directory)
	{
		return !Directory.UserPath.Path.IsEmpty() && Directory.bIsDefault;
	});

	if (DefaultUserDirectory == nullptr)
		return false;

	FString UserCacheFileName = FPaths::Combine(DefaultUserDirectory->UserPath.Path, TEXT("Cache"), GraphID + TEXT(".png"));
	FPaths::NormalizeFilename(UserCacheFileName);

	return FPaths::FileExists(UserCacheFileName);
}

TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> FInstaMAT::GetGraphObjectLibraryPreviews(bool bEnforceRecache)
{
	check(InstaMAT != nullptr);

	static TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> Array;

	FScopeLock Lock(&Mutex);
	if (Array.Num() == 0 || bEnforceRecache)
	{
		Array.Empty();

		const TArray<FString>& CategoryArray = GetCategories(/*bEnforceRecache:*/ true);
		TMap<FString, TSharedPtr<FInstaMATGraphObjectViewItem>> CheckDoubleMap;

		for (const FString& Category : CategoryArray)
		{
			InstaMAT::IGraphObject** Objects = nullptr;
			InstaMAT->GetGraphObjectsInCategory(TCHAR_TO_UTF8(*Category), &Objects);

			for (auto GraphObjectIterator = Objects; *GraphObjectIterator != nullptr; GraphObjectIterator++)
			{
				const InstaMAT::IGraphObject* const GraphObject = *GraphObjectIterator;

				if (GraphObject == nullptr)
					continue;

				if (!InstaMATGraphUtility::IsGraphObjectValid(GraphObject))
					continue;

				bool bIsFromUserPackage = false;
				
				if (const InstaMAT::IGraphPackage* const Package = GraphObject->GetParentPackage())
				{
					bIsFromUserPackage = kInstaMATUserPackageType == FString(UTF8_TO_TCHAR(Package->GetOriginType()));
				}

				InstaMAT::IElementExecution* const Execution = InstaMAT->AllocElementExecution();

				ON_SCOPE_EXIT
				{
					if (Execution != nullptr)
					{
						InstaMAT->DeallocElementExecution(Execution);
					}
				};

				TSharedPtr<FInstaMATGraphObjectViewItem> PreviewItem(new FInstaMATGraphObjectViewItem);

				const ANSICHAR* const FriendlyName = GraphObject->GetName(/*friendlyName:*/ true);
				PreviewItem->GraphFriendlyName = FString(UTF8_TO_TCHAR(FriendlyName));

				const ANSICHAR* const Name = GraphObject->GetName(/*friendlyName:*/ false);
				PreviewItem->GraphName = FString(UTF8_TO_TCHAR(Name));

				const uint32 BufferSize = 128u;
				ANSICHAR Buffer[BufferSize];

				const uint32 WrittenSize = GraphObject->GetID(Buffer, BufferSize);
				PreviewItem->GraphID = FString(UTF8_TO_TCHAR(Buffer));
				PreviewItem->Category = Category;

				// make graphs are only shown once
				if (CheckDoubleMap.Contains(PreviewItem->GraphID))
				{
					// check which category is longer
					const TSharedPtr<FInstaMATGraphObjectViewItem>& Other = CheckDoubleMap[PreviewItem->GraphID];

					if (Other->Category.Len() > PreviewItem->Category.Len())
						continue;

					Array.Remove(Other);
				}

				CheckDoubleMap.Add(PreviewItem->GraphID, PreviewItem);
				Array.Add(PreviewItem);

				uint32 Major = 0u;
				uint32 Minor = 0u;
				GraphObject->GetVersion(Major, Minor);

				PreviewItem->Version = FString::Printf(TEXT("%u.%u"), Major, Minor);

				if (GraphObject->ContainsMetaDataKeyChar(InstaMAT::MetaData::KeyAuthor))
				{
					PreviewItem->Author = GraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyAuthor);
				}
				if (GraphObject->ContainsMetaDataKeyChar(InstaMAT::MetaData::KeyURL))
				{
					PreviewItem->URL = GraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyURL);
				}
				if (GraphObject->ContainsMetaDataKeyChar(InstaMAT::MetaData::KeyDocumentation))
				{
					PreviewItem->Documentation = GraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyDocumentation);
				}
				if (GraphObject->ContainsMetaDataKeyChar(kInstaMATMetaDataInputParameterCategoryOrder))
				{
					const FString InputCategories = GraphObject->GetMetaDataAsChar(kInstaMATMetaDataInputParameterCategoryOrder);
					InputCategories.ParseIntoArrayWS(PreviewItem->InputCategories, TEXT(";"));
					PreviewItem->InputCategories.AddUnique(FString(TEXT("Input")));
				}
				if (GraphObject->ContainsMetaDataKeyChar(kInstaMATMetaDataTags))
				{
					PreviewItem->Tags = GraphObject->GetMetaDataAsChar(kInstaMATMetaDataTags);
				}

				PreviewItem->bIsUserGraph = bIsFromUserPackage;

				if (const InstaMAT::IGraph* const Graph = GraphObject->AsGraph())
				{
					const uint32 InputParameterCount = Graph->GetParameterCount(InstaMAT::IGraph::ParameterTypeInput);

					for (uint32 ParameterIndex = 0u; ParameterIndex < InputParameterCount; ParameterIndex++)
					{
						const InstaMAT::IGraphVariable* const InputGraphVariable = Graph->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeInput);
						const InstaMAT::IGraphVariable::Type VariableType = InputGraphVariable->GetVariableTypeValue();

						if (VariableType == InstaMAT::IGraphVariable::Type::TypeElementResource ||
							VariableType == InstaMAT::IGraphVariable::Type::TypeElementPointCloud ||
							VariableType == InstaMAT::IGraphVariable::Type::TypeElementScene)
							continue;
						
						if (const InstaMAT::IGraphObject* const VariableGraphObject = InputGraphVariable->AsObject())
						{
							PreviewItem->InputDefinitions.Add(FInstaMATGraphObjectInputData());
							FInstaMATGraphObjectInputData& Input = PreviewItem->InputDefinitions.Last();

							const FString InputCategory = FString(UTF8_TO_TCHAR(VariableGraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyCategory)));

							Input.Name = FString(UTF8_TO_TCHAR(VariableGraphObject->GetName(/*friendlyName:*/ true)));
							Input.TypeString = FString(UTF8_TO_TCHAR(InputGraphVariable->GetVariableTypeString()));
							Input.Documentation = FString(UTF8_TO_TCHAR(VariableGraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyDocumentation)));
							Input.Category = InputCategory.IsEmpty() ? FString(TEXT("Input")) : InputCategory;
							Input.Type = VariableType;
							Input.DefaultValue = InputGraphVariable->GetArithmeticValue();
						}
					}

					const uint32 OutputParameterCount = Graph->GetParameterCount(InstaMAT::IGraph::ParameterTypeOutput);

					for (uint32 ParameterIndex = 0u; ParameterIndex < OutputParameterCount; ParameterIndex++)
					{
						const InstaMAT::IGraphVariable* const OutputGraphVariable = Graph->GetParameterAtIndex(ParameterIndex, InstaMAT::IGraph::ParameterTypeOutput);
						const InstaMAT::IGraphVariable::Type VariableType = OutputGraphVariable->GetVariableTypeValue();
						
						if (const InstaMAT::IGraphObject* const VariableGraphObject = OutputGraphVariable->AsObject())
						{
							PreviewItem->OutputDefinitions.Add(FInstaMATGraphObjectOutputData());
							FInstaMATGraphObjectOutputData& Output = PreviewItem->OutputDefinitions.Last();
							Output.Type = VariableType;
							Output.Name = FString(UTF8_TO_TCHAR(VariableGraphObject->GetName(/*friendlyName:*/ true)));
							Output.Documentation = FString(UTF8_TO_TCHAR(VariableGraphObject->GetMetaDataAsChar(InstaMAT::MetaData::KeyDocumentation)));
							Output.TypeString = FString(UTF8_TO_TCHAR(OutputGraphVariable->GetVariableTypeString()));

							if (VariableType == InstaMAT::IGraphVariable::Type::TypeAtomOutputImage ||
								VariableType == InstaMAT::IGraphVariable::Type::TypeAtomOutputImageGray ||
								VariableType == InstaMAT::IGraphVariable::Type::TypeElementImage ||
								VariableType == InstaMAT::IGraphVariable::Type::TypeElementImageGray)
							{
								Output.ColorSpace = OutputGraphVariable->GetColorSpaceTypeValue();
							}
						}
					}
				}

				// clear preview image
				PreviewItem->Preview = nullptr;
			}
		}
	}
	Lock.Unlock();

	// return a copy
	return Array;
}

TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> FInstaMAT::FindGraphObjectWithName(const FString& Name)
{
	if (Name.IsEmpty())
		return TArray<TSharedPtr<FInstaMATGraphObjectViewItem>>();

	TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> Result;
	TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> Library = GetGraphObjectLibraryPreviews();

	const FString NameLowercase = Name.ToLower();

	for (const TSharedPtr<FInstaMATGraphObjectViewItem>& Item : Library)
	{
		if (Item->GraphFriendlyName.ToLower() == NameLowercase ||
			Item->GraphName.ToLower() == NameLowercase)
		{
			Result.Add(Item);
		}
	}

	return Result;
}

bool FInstaMAT::IsAsyncOperationInProgress()
{
	return InstaMATThread->IsBusy() || FInstaMAT::bIsAsyncProcessRunning;
}

void FInstaMAT::SetAsyncOperationInProgress(const bool bIsExecuting)
{
	FInstaMAT::bIsAsyncProcessRunning = bIsExecuting;
}

float* FInstaMAT::GetProgressValue()
{
	static float ProgressValue = 0.0f;

	return &ProgressValue;
}

int64 FInstaMAT::GetDefaultVRAMBudget()
{
	check(InstaMAT != nullptr);

	const int64 AvailableMemory = InstaMAT->GetTotalAvailableVideoMemory();
	const int64 kKiloByte = 1024;
	const int64 kGigaByte = kKiloByte * kKiloByte * kKiloByte;

	// Not enough memory for custom settings
	if (AvailableMemory <= kGigaByte)
		return -1;

	return FMath::Max(kGigaByte, AvailableMemory / 2);
}

#undef LOCTEXT_NAMESPACE
