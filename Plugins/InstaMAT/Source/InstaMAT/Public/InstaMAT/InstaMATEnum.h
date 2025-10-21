/**
 * InstaMATEnum.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATEnum.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InstaMATEnum.generated.h"

/**
 * The EInstaMATTextureSize Enum describes all valid 
 * resolutions for texture output.
 */
UENUM()
enum class EInstaMATTextureSize : uint8
{
	InstaMAT_128		UMETA(DisplayName = "128"),
	InstaMAT_256		UMETA(DisplayName = "256"),
	InstaMAT_512		UMETA(DisplayName = "512"),
	InstaMAT_1024		UMETA(DisplayName = "1024"),
	InstaMAT_2K			UMETA(DisplayName = "2k"),
	InstaMAT_4K			UMETA(DisplayName = "4k"),
	InstaMAT_8K			UMETA(DisplayName = "8k")
};

/**
 * The EInstaMATTextureFileType Enum defines 
 * image export types.
 */
UENUM()
enum class EInstaMATTextureFileType : uint8
{
	InstaMAT_PNG		UMETA(DisplayName = "PNG"),
	InstaMAT_JPG		UMETA(DisplayName = "JPG"),
	InstaMAT_HDR		UMETA(DisplayName = "HDR"),
	InstaMAT_BMP		UMETA(DisplayName = "BMP"),
	InstaMAT_TIFF		UMETA(DisplayName = "TIFF"),
	InstaMAT_TGA		UMETA(DisplayName = "TGA"),
	InstaMAT_EXR		UMETA(DisplayName = "EXR")
};

/**
 * The EInstaMATExecutionFormat Enum defines the execution
 * format of the instance.
 */
UENUM()
enum class EInstaMATExecutionFormat : uint8
{
	InstaMAT_Normalized8	UMETA(DisplayName = "Normalized 8 Bit"),
	InstaMAT_Normalized16	UMETA(DisplayName = "Normalized 16 Bit"),
	InstaMAT_FullRange16	UMETA(DisplayName = "FullRange 16 Bit"),
	InstaMAT_FullRange32	UMETA(DisplayName = "FullRange 32 Bit")
};

/**
 * The EInstaMATRotation Enum defines a set of fixed rotation values.
 */
UENUM()
enum class EInstaMATRotation : uint8
{
	InstaMAT_Rotation0		UMETA(DisplayName = "0° Rotation"),
	InstaMAT_Rotation90		UMETA(DisplayName = "90° Rotation"),
	InstaMAT_Rotation180	UMETA(DisplayName = "180° Rotation"),
	InstaMAT_Rotation270	UMETA(DisplayName = "270° Rotation")
};

/**
 * The EInstaMATLibaryFilter Enum defines filter options for the library.
 */
UENUM()
enum class EInstaMATLibraryFilter : uint8
{
	InstaMAT_Material		UMETA(DisplayName = "Default"),
	InstaMAT_Mesh			UMETA(DisplayName = "Mesh"),
	InstaMAT_User			UMETA(DisplayName = "User Packages"),
	InstaMAT_All			UMETA(DisplayName = "All")
};

/**
 * The EInstaMATLibraryGraphListRowType Enum defines render 
 * types for the graph list in the library.
 */
UENUM()
enum class EInstaMATLibraryGraphListRowType : uint8
{
	InstaMAT_ListSmallIcon		UMETA(DisplayName = "List with smaller icons"),
	InstaMAT_TileMediumIcon		UMETA(DisplayName = "Tile with medium icons"),
	InstaMAT_TileBigIcon		UMETA(DisplayName = "Tile with big icons")
};

/**
 * The EInstaMATUpdateType Enum defines the update type for an graph instance.
 */
UENUM()
enum class EInstaMATUpdateType : uint8
{
	InstaMAT_Manual			UMETA(DisplayName = "Manual"),
	InstaMAT_Automatic		UMETA(DisplayName = "Automatic")
};

namespace InstaMAT
{
	/**
	 * Gets the rotation in radians for the specified value.
	 * 
	 * @param Value the enum value.
	 * @return The rotation in radians.
	 */
	FORCEINLINE float GetRotationInRadiansForInstaMATRotationEnum(const EInstaMATRotation Value)
	{
		return UE_HALF_PI * (float)Value;
	}
}