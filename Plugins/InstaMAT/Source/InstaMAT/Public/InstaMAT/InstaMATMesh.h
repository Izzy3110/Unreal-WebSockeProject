/**
 * InstaMATMesh.h (InstaMAT)
 *
 * Copyright 2019-2021 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATMesh.h
 * @copyright 2019-2021 InstaMaterial GmbH. All rights reserved.
 * @section License
 */
#pragma once

#ifndef INSTAMAT_INSTAMATMESH_h
#define INSTAMAT_INSTAMATMESH_h

#include "CoreMinimal.h"
#include "InstaMATAPI.h"

/**
 * The InstaMATMesh class extends the abstract IGraphMesh class. 
 */
class INSTAMAT_API InstaMATMesh : public InstaMAT::IGraphMesh
{
public:

	InstaMATMesh() {}
	virtual ~InstaMATMesh() {}

	/**
	 * Gets the mesh vertices.
	 *
	 * @return The mesh vertices.
	 */
	virtual InstaMAT::GraphMeshVertex* GetVertices() override
	{
		return Vertices.GetData();
	}

	/**
	 * Gets the mesh vertices.
	 *
	 * @return The mesh vertices.
	 */
	virtual const InstaMAT::GraphMeshVertex* GetVertices() const override
	{
		return Vertices.GetData();
	}

	/**
	 * Gets the mesh indices.
	 *
	 * @return The mesh indices.
	 */
	virtual InstaMAT::uint32* GetIndices() override
	{
		return Indices.GetData();
	}

	/**
	 * Gets the mesh indices.
	 *
	 * @return The mesh indices.
	 */
	virtual const InstaMAT::uint32* GetIndices() const override
	{
		return Indices.GetData();
	}

	/**
	 * Gets the submesh mesh indices.
	 * @note Submesh must be in ascending order.
	 * If the criteria does not match, the mesh will need to be preprocessed.
	 * @note Submesh indices denote sub-mesh groups in a larger buffer.
	 * Submesh could be to separate meshes by material, or to
	 * distincly identify polygon groups on the mesh.
	 * @note The number of submesh indices must match
	 * the polygon count of the mesh, which is `IndexCount / 3`.
	 * @note If the mesh does not have submeshes, simply set each
	 * element of the array to 0.
	 *
	 * @return The submesh indices.
	 */
	virtual InstaMAT::uint32* GetSubmeshIndices() override
	{
		return SubmeshIndices.GetData();
	}

	/**
	 * Gets the submesh mesh indices.
	 * @note Submesh must be in ascending order.
	 * If the criteria does not match, the mesh will need to be preprocessed.
	 * @note Submesh indices denote sub-mesh groups in a larger buffer.
	 * Submesh could be to separate meshes by material, or to
	 * distincly identify polygon groups on the mesh.
	 * @note The number of submesh indices must match
	 * the polygon count of the mesh, which is `IndexCount / 3`.
	 * @note If the mesh does not have submeshes, simply set each
	 * element of the array to 0.
	 *
	 * @return The submesh indices.
	 */
	virtual const InstaMAT::uint32* GetSubmeshIndices() const override
	{
		return SubmeshIndices.GetData();
	}

	/**
	 * Gets the vertex count.
	 *
	 * @return Vertex count.
	 */
	virtual InstaMAT::uint32 GetVertexCount() const override
	{
		return Vertices.Num();
	}

	/**
	 * Gets the index count.
	 *
	 * @return Index count.
	 */
	virtual InstaMAT::uint32 GetIndexCount() const override
	{
		return Indices.Num();
	}

	/**
	 * Gets the submesh name.
	 * @note Each submesh index in the SubmeshIndices array
	 * must return a valid name.
	 *
	 * @param SubmeshIndex The submesh index.
	 * @return The submesh name.
	 */
	virtual const char* GetSubmeshName(const InstaMAT::uint32 SubmeshIndex) const override
	{
		static thread_local char SubMeshName[128];
		FMemory::Memset(&SubMeshName, 0, sizeof(SubMeshName));
		snprintf(SubMeshName, sizeof(SubMeshName), "%u", SubmeshIndex);
		return SubMeshName;
	}

	/**
	 * Gets the Indices array.
	 * 
	 * @return reference to the Indices array.
	 */
	TArray<InstaMAT::uint32>& GetIndicesArray() 
	{ 
		return Indices;
	}

	/**
	 * Gets the SubmeshIndices array.
	 *
	 * @return reference to the SubmeshIndices array.
	 */
	TArray<InstaMAT::uint32>& GetSubmeshIndicesArray()
	{
		return SubmeshIndices;
	}

	/**
	 * Gets the Vertices array.
	 *
	 * @return reference to the Vertices array.
	 */
	TArray<InstaMAT::GraphMeshVertex>& GetVerticesArray()
	{
		return Vertices;
	}

	/**
	 * Gets the MaterialIndices array.
	 *
	 * @return reference to the MaterialIndices array.
	 */
	TArray<InstaMAT::uint32>& GetMaterialIndicesArray()
	{
		return MaterialIndices;
	}

	/**
	 * Gets the material indices.
	 * @note Material indices denote the material assigned to a face .
	 * @note The number of material indices must match
	 * the polygon count of the mesh, which is `IndexCount / 3`.
	 *
	 * @return The material indices.
	 */
	virtual uint32* GetMaterialIndices() override
	{
		return MaterialIndices.GetData();
	}

	/**
	 * Gets the material indices.
	 * @note Material indices denote the material assigned to a face .
	 * @note The number of material indices must match
	 * the polygon count of the mesh, which is `IndexCount / 3`.
	 *
	 * @return The material indices.
	 */
	virtual const uint32* GetMaterialIndices() const override
	{
		return MaterialIndices.GetData();
	}

	/**
	 * Gets the material name for \p materialIndex
	 * @note All values up to max(materialIndex) in the MaterialIndices array
	 * must return a valid name.
	 * Example: If the maximum material index is 12, then material
	 * indices 0..12 must return valid names, even if only 12 is used.
	 *
	 * @param MaterialIndex The material index.
	 * @return The material name.
	 */
	virtual const char* GetMaterialName(const uint32 MaterialIndex) const override
	{
		static thread_local char SubMeshName[128];
		FMemory::Memset(&SubMeshName, 0, sizeof(SubMeshName));
		snprintf(SubMeshName, sizeof(SubMeshName), "%u", MaterialIndex);
		return SubMeshName;
	}

protected:

	TArray<InstaMAT::uint32> Indices;			/**< The Indices of this mesh. */	
	TArray<InstaMAT::uint32> SubmeshIndices;	/**< The Submesh Indices of this mesh. */
	TArray<InstaMAT::uint32> MaterialIndices;	/**< The Material Indices of this mesh. */
	TArray<InstaMAT::GraphMeshVertex> Vertices;	/**< The Vertices of this mesh. */
};

#endif /* INSTAMAT_INSTAMATMESH_h */