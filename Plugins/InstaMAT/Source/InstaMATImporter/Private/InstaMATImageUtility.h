/**
 * InstaMATImageUtility.h (InstaMAT)
 *
 * Copyright 2019-2023 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImageUtility.h
 * @copyright 2019-2023 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "InstaMATTextureDataSource.h"

namespace InstaMATImageUtility
{
	/**
	 * Gets the bit depth for the specified \p ExecutionFormat.
	 *
	 * @param ExecutionFormat the execution format.
	 * @return The bit depth.
	 */
	FORCEINLINE static uint32 GetBitDepthForExecutionFormat(const EInstaMATExecutionFormat ExecutionFormat)
	{
		switch (ExecutionFormat)
		{
		case EInstaMATExecutionFormat::InstaMAT_Normalized8:
			return 8u;
		case EInstaMATExecutionFormat::InstaMAT_Normalized16:
		case EInstaMATExecutionFormat::InstaMAT_FullRange16:
			return 16u;
		case EInstaMATExecutionFormat::InstaMAT_FullRange32:
			return 32u;
		default:
			check(false)
		}
		return 8u;
	}

	/**
	 * Returns the number value for the specified \p Resolution.
	 *
	 * @param Resolution the resolution enum value
	 * @return The resolution in pixel.
	 */;
	FORCEINLINE static uint32 TextureResolutionEnumToUInt32(const EInstaMATTextureSize Resolution)
	{
		switch (Resolution)
		{
		case EInstaMATTextureSize::InstaMAT_128:
			return 128u;
		case EInstaMATTextureSize::InstaMAT_256:
			return 256u;
		case EInstaMATTextureSize::InstaMAT_512:
			return 512u;
		case EInstaMATTextureSize::InstaMAT_1024:
			return 1024u;
		case EInstaMATTextureSize::InstaMAT_2K:
			return 2048u;
		case EInstaMATTextureSize::InstaMAT_4K:
			return 4096u;
		case EInstaMATTextureSize::InstaMAT_8K:
			return 8192u;
		default:
			check(false);
		}
		return 1024u;
	}

	/**
	* Returns the component block size e for the specified \p Format.
	* 
	* @param Format the execution format
	* @return The block size in bytes.
	*/
	FORCEINLINE static uint32 BlockSizeForExecutionFormat(EInstaMATExecutionFormat Format)
	{
		switch (Format)
		{
		case EInstaMATExecutionFormat::InstaMAT_Normalized8:
			return 1u;
		case EInstaMATExecutionFormat::InstaMAT_Normalized16:
		case EInstaMATExecutionFormat::InstaMAT_FullRange16:
			return 2u;
			break;
		case EInstaMATExecutionFormat::InstaMAT_FullRange32:
			return 4u;
		}
		return 0u;
	}

	/**
	* Returns true if a pixel format is a compressed texture.
	*
	* @param Format the pixel format
	* @return True if format is a compressed texture.
	*/
	FORCEINLINE static bool IsCompressedPixelFormat(EPixelFormat Format)
	{
		return Format == EPixelFormat::PF_DXT1 ||
			Format == EPixelFormat::PF_DXT3 ||
			Format == EPixelFormat::PF_DXT5 ||
			Format == EPixelFormat::PF_BC4 ||
			Format == EPixelFormat::PF_BC5 ||
			Format == EPixelFormat::PF_BC7 ||
			Format == EPixelFormat::PF_BC6H;
	}

	/**
	* Returns GPU pixel format for an execution format.
	*
	* @param Format the execution format
	* @param ChannelCount the channel count of an image
	* @param bBGRA Prefer BGRA for N8 format, instead of RGBA
	* @return The pixel format.
	*/
	FORCEINLINE static EPixelFormat PixelFormatFromExecutionFormat(EInstaMATExecutionFormat Format, uint32 ChannelCount, const bool bBGRA = true)
	{
		switch (Format)
		{
		case EInstaMATExecutionFormat::InstaMAT_Normalized8:
			if (ChannelCount == 1u)
				return EPixelFormat::PF_G8;
			else if (ChannelCount == 4u)
				return bBGRA ? EPixelFormat::PF_B8G8R8A8 : EPixelFormat::PF_R8G8B8A8;
			break;
		case EInstaMATExecutionFormat::InstaMAT_Normalized16:
			if (ChannelCount == 1u)
				return EPixelFormat::PF_G16;
			else if (ChannelCount == 4u)
				return EPixelFormat::PF_R16G16B16A16_UINT;
			break;
		case EInstaMATExecutionFormat::InstaMAT_FullRange16:
			//TODO: support FP16
		case EInstaMATExecutionFormat::InstaMAT_FullRange32:
			if (ChannelCount == 1u)
				return EPixelFormat::PF_R32_FLOAT;
			else if (ChannelCount == 4u)
				return EPixelFormat::PF_A32B32G32R32F;
			break;
		default:
			check(false);
		}
		return EPixelFormat::PF_Unknown;
	}

	/**
	* Returns an execution format from a GPU pixel format.
	*
	* @param Format the pixel format
	* @return The execution format.
	*/
	FORCEINLINE static EInstaMATExecutionFormat ExecutionFormatFromPixelFormat(EPixelFormat Format)
	{
		switch (Format)
		{
		case EPixelFormat::PF_G8:
		case EPixelFormat::PF_B8G8R8A8:
		case EPixelFormat::PF_R8G8B8A8:
			return EInstaMATExecutionFormat::InstaMAT_Normalized8;
		case EPixelFormat::PF_G16:
		case EPixelFormat::PF_R16G16B16A16_UINT:
			return EInstaMATExecutionFormat::InstaMAT_Normalized16;
		case EPixelFormat::PF_R32_FLOAT:
		case EPixelFormat::PF_A32B32G32R32F:
			return EInstaMATExecutionFormat::InstaMAT_FullRange32;
		default:
			check(false);
		}
		return EInstaMATExecutionFormat::InstaMAT_Normalized8;
	}

	/**
	* Returns channel count from a GPU pixel format.
	*
	* @param Format the pixel format
	* @return The channel count.
	*/
	FORCEINLINE static uint32 ChannelCountForPixelFormat(EPixelFormat Format)
	{
		switch (Format)
		{
		case EPixelFormat::PF_G8:
		case EPixelFormat::PF_G16:
		case EPixelFormat::PF_R32_FLOAT:
			return 1u;
		case EPixelFormat::PF_B8G8R8A8:
		case EPixelFormat::PF_R8G8B8A8:
		case EPixelFormat::PF_R16G16B16A16_UINT:
		case EPixelFormat::PF_A32B32G32R32F:
			return 4u;
		default:
			check(false);
		}
		return 0u;
	}

	/**
	* Returns a texture source format for an execution format.
	*
	* @param Format the execution format
	* @param ChannelCount the channel count of an image
	* @return The texture source format.
	*/
	FORCEINLINE static ETextureSourceFormat TextureSourceFormatFromExecutionFormat(EInstaMATExecutionFormat Format, uint32 ChannelCount)
	{
		switch (Format)
		{
		case EInstaMATExecutionFormat::InstaMAT_Normalized8:
			if (ChannelCount == 1u)
				return ETextureSourceFormat::TSF_G8;
			else if (ChannelCount == 4u)
				// we always use BGRA for 8 bit
				return ETextureSourceFormat::TSF_BGRA8;
			break;
		case EInstaMATExecutionFormat::InstaMAT_Normalized16:
			if (ChannelCount == 1u)
				return ETextureSourceFormat::TSF_G16;
			else if (ChannelCount == 4u)
				return ETextureSourceFormat::TSF_RGBA16;
			break;
		case EInstaMATExecutionFormat::InstaMAT_FullRange16:
			//TODO: support FP16
		case EInstaMATExecutionFormat::InstaMAT_FullRange32:
			if (ChannelCount == 1u)
				return ETextureSourceFormat::TSF_R32F;
			else if (ChannelCount == 4u)
				return ETextureSourceFormat::TSF_RGBA32F;
			break;
		default:
			check(false);
		}
		check(false);
		return ETextureSourceFormat::TSF_Invalid;
	}

	/**
	 * Converts from n-channel RGBA N8/N16 format to 4-channel BGRA N8.
	 *
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param ChannelCount The image component count.
	 * @param SourceBlockSize The block size of the source image (e.g. 1 for N8 and 2 for N16).
	 * @param Source The source image data.
	 * @param Destination The destination image data.
	 * @param bSRGB Apply Linear to sRGB conversion.
	 */
	inline static void ConvertFromRGBAToBGRA8(const uint32 Width, const uint32 Height, uint32 ChannelCount, uint32 SourceBlockSize, const uint8* const Source, TArray<uint8>& Destination, bool bSRGB = false)
	{
		check(SourceBlockSize == 1u || SourceBlockSize == 2u);
		if (ChannelCount == 1u)
			return;

		Destination.SetNumZeroed(Width * Height * 4u);

		static const float kMaxUint8 = (float)TNumericLimits<uint8>::Max();
		static const float kInverseMaxUint8 = 1.0f / kMaxUint8;

		uint8* const DestinationData = Destination.GetData();
		if (SourceBlockSize == 1u)
		{
			ParallelFor(Height, [DestinationData, Source, Width, ChannelCount, bSRGB](int32 Y)
			{
				const uint64 RowStart = (uint64)Y * Width;
				for (uint32 X = 0u; X < Width; X++)
				{
					const uint64 Offset = RowStart + X;
					const uint64 SourceIndex = Offset * ChannelCount;
					const uint64 DestinationIndex = Offset * 4u;
					// B - R
					DestinationData[DestinationIndex] = ChannelCount == 2u ? Source[SourceIndex] : Source[SourceIndex + 2u];
					if (ChannelCount >= 3u)
					{
						// RGB and RGBA only
						// G - G
						DestinationData[DestinationIndex + 1u] = Source[SourceIndex + 1u];
						// R - B
						DestinationData[DestinationIndex + 2u] = Source[SourceIndex];
					}
					// A
					DestinationData[DestinationIndex + 3u] = ChannelCount == 3u ? TNumericLimits<uint8>::Max() : Source[SourceIndex + ChannelCount - 1u];
					if (bSRGB)
					{
						DestinationData[DestinationIndex] = uint8(FMath::Pow(DestinationData[DestinationIndex] * kInverseMaxUint8, 1.0f / 2.2f) * kMaxUint8);
						DestinationData[DestinationIndex + 1u] = uint8(FMath::Pow(DestinationData[DestinationIndex + 1u] * kInverseMaxUint8, 1.0f / 2.2f) * kMaxUint8);
						DestinationData[DestinationIndex + 2u] = uint8(FMath::Pow(DestinationData[DestinationIndex + 2u] * kInverseMaxUint8, 1.0f / 2.2f) * kMaxUint8);
						DestinationData[DestinationIndex + 3u] = uint8(FMath::Pow(DestinationData[DestinationIndex + 3u] * kInverseMaxUint8, 1.0f / 2.2f) * kMaxUint8);
					}
				}
			}, EParallelForFlags::None);
		}
		else
		{
			const uint16* const SourceData = reinterpret_cast<const uint16*>(Source);
			ParallelFor(Height, [DestinationData, SourceData, Width, ChannelCount, bSRGB](int32 Y)
			{
				const uint64 RowStart = (uint64)Y * Width;
				for (uint32 X = 0u; X < Width; X++)
				{
					const uint64 Offset = RowStart + X;
					const uint64 SourceIndex = Offset * ChannelCount;
					const uint64 DestinationIndex = Offset * 4u;
					// B - R
					DestinationData[DestinationIndex] = ChannelCount == 2u ? SourceData[SourceIndex] >> 8 : SourceData[SourceIndex + 2u] >> 8;
					if (ChannelCount >= 3u)
					{
						// RGB and RGBA only
						// G - G
						DestinationData[DestinationIndex + 1u] = SourceData[SourceIndex + 1u] >> 8;
						// R - B
						DestinationData[DestinationIndex + 2u] = SourceData[SourceIndex] >> 8;
					}
					// A
					DestinationData[DestinationIndex + 3u] = ChannelCount == 3u ? TNumericLimits<uint8>::Max() : SourceData[SourceIndex + ChannelCount - 1u] >> 8;
					if (bSRGB)
					{
						DestinationData[DestinationIndex] = uint8(FMath::Pow(DestinationData[DestinationIndex] * kInverseMaxUint8, 1.0f / 2.2f) * kMaxUint8);
						DestinationData[DestinationIndex + 1u] = uint8(FMath::Pow(DestinationData[DestinationIndex + 1u] * kInverseMaxUint8, 1.0f / 2.2f) * kMaxUint8);
						DestinationData[DestinationIndex + 2u] = uint8(FMath::Pow(DestinationData[DestinationIndex + 2u] * kInverseMaxUint8, 1.0f / 2.2f) * kMaxUint8);
						DestinationData[DestinationIndex + 3u] = uint8(FMath::Pow(DestinationData[DestinationIndex + 3u] * kInverseMaxUint8, 1.0f / 2.2f) * kMaxUint8);
					}
				}
			}, EParallelForFlags::None);
		}
	}

	/**
	 * Applies a gamma correction for an N8 or N16 image.
	 * 
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param ChannelCount The image component count.
	 * @param DataBlockSize The block size of the source image (e.g. 1 for N8 and 2 for N16).
	 * @param Gamma The gamma value to apply.
	 * @param Data The image data to modify.
	 */
	inline static void ApplyGamma(const uint32 Width, const uint32 Height, const uint32 ChannelCount, const uint32 DataBlockSize, const float Gamma, uint8* const Data)
	{
		check(DataBlockSize == 1u || DataBlockSize == 2u);
		if (DataBlockSize == 1u)
		{
			static const float kMaxUint8 = (float)TNumericLimits<uint8>::Max();
			static const float kInverseMaxUint8 = 1.0f / kMaxUint8;
			ParallelFor(Height, [Data, Width, ChannelCount, Gamma](int32 y)
			{
				const uint64 RowStart = (uint64)y * Width;
				for (uint32 x = 0u; x < Width; x++)
				{
					const uint64 Offset = RowStart + x;
					const uint64 Index = Offset * ChannelCount;
					for (uint32 c = 0u; c < ChannelCount; c++)
					{
						Data[Index + c] = uint8(FMath::Pow(Data[Index + c] * kInverseMaxUint8, Gamma) * kMaxUint8);
					}
				}
			}, EParallelForFlags::None);
		}
		else
		{
			static const float kMaxUint16 = (float)TNumericLimits<uint16>::Max();
			static const float kInverseMaxUint16 = 1.0f / kMaxUint16;
			uint16* const Data16 = reinterpret_cast<uint16*>(Data);
			ParallelFor(Height, [Data16, Width, ChannelCount, Gamma](int32 Y)
			{
				const uint64 RowStart = (uint64)Y * Width;
				for (uint32 X = 0u; X < Width; X++)
				{
					const uint64 Offset = RowStart + X;
					const uint64 Index = Offset * ChannelCount;
					for (uint32 C = 0u; C < ChannelCount; C++)
					{
						Data16[Index + C] = uint16(FMath::Pow(Data16[Index + C] * kInverseMaxUint16, Gamma) * kMaxUint16);
					}
				}
			}, EParallelForFlags::None);
		}
	}

	/**
	 * Converts an image from linear to sRGB space, only for N8 or N16 images.
	 *
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param ChannelCount The image component count.
	 * @param DataBlockSize The block size of the source image (e.g. 1 for N8 and 2 for N16).
	 * @param Data The image data to modify.
	 */
	FORCEINLINE static void ConvertLinearToSRGB(const uint32 Width, const uint32 Height, const uint32 ChannelCount, const uint32 DataBlockSize, uint8* const Data)
	{
		ApplyGamma(Width, Height, ChannelCount, DataBlockSize, 1.0f / 2.2f, Data);
	}

	/**
	 * Converts an image from sRGB to linear space, only for N8 or N16 images.
	 *
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param ChannelCount The image component count.
	 * @param DataBlockSize The block size of the source image (e.g. 1 for N8 and 2 for N16).
	 * @param Data The image data to modify.
	 */
	FORCEINLINE static void ConvertSRGBToLinear(const uint32 Width, const uint32 Height, const uint32 ChannelCount, const uint32 DataBlockSize, uint8* const Data)
	{
		ApplyGamma(Width, Height, ChannelCount, DataBlockSize, 2.2f, Data);
	}

	/**
	 * Converts from n-channel RGBA N16 format to 4-channel RGBA N16.
	 *
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param ChannelCount The image component count.
	 * @param Source The source image data.
	 * @param Destination The destination image data.
	 */
	inline static void ConvertToRGBA_N16(const uint32 Width, const uint32 Height, uint32 ChannelCount, const uint8* const Source, TArray<uint8>& Destination)
	{
		if (ChannelCount == 1u || ChannelCount == 4u)
			return;

		Destination.SetNumZeroed(Width * Height * 4u * sizeof(uint16));

		const uint16* const SourceData = reinterpret_cast<const uint16*>(Source);
		uint16* const DestinationData = reinterpret_cast<uint16*>(Destination.GetData());
		ParallelFor(Height, [DestinationData, SourceData, Width, ChannelCount](const uint32 Y)
		{
			const uint64 RowStart = Y * Width;
			for (uint32 X = 0u; X < Width; X++)
			{
				const uint64 Offset = RowStart + X;
				const uint64 SourceIndex = Offset * ChannelCount;
				const uint64 DestinationIndex = Offset * 4u;

				memcpy(&DestinationData[DestinationIndex], &SourceData[SourceIndex], sizeof(uint16) * ChannelCount);
				// A
				DestinationData[DestinationIndex + 3u] = TNumericLimits<uint16>::Max();
			}
		});
	}

	/**
	 * Converts from n-channel RGBA F32 format to 4-channel RGBA F32.
	 * 
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param ChannelCount The image component count.
	 * @param Source The source image data.
	 * @param Destination The destination image data.
	 */
	inline static void ConvertToRGBA_F32(const uint32 Width, const uint32 Height, uint32 ChannelCount, const uint8* const Source, TArray<uint8>& Destination)
	{
		if (ChannelCount == 1u || ChannelCount == 4u)
			return;

		Destination.SetNumZeroed(Width * Height * 4u * sizeof(float));

		const float* const SourceData = reinterpret_cast<const float*>(Source);
		float* const DestinationData = reinterpret_cast<float*>(Destination.GetData());
		ParallelFor(Height, [DestinationData, SourceData, Width, ChannelCount](const uint32 Y)
		{
			const uint64 RowStart = Y * Width;
			for (uint32 X = 0u; X < Width; X++)
			{
				const uint64 Offset = RowStart + X;
				const uint64 SourceIndex = Offset * ChannelCount;
				const uint64 DestinationIndex = Offset * 4u;

				memcpy(&DestinationData[DestinationIndex], &SourceData[SourceIndex], sizeof(float) * ChannelCount);
				// A
				DestinationData[DestinationIndex + 3u] = 1.0f;
			}
		});
	}
}

namespace InstaMATNormalMapUtility
{
	/**
	 * Applies a rotation on the specified \p OutBitmap.
	 *
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param Rotation The rotation.
	 * @param [out] OutBitmap Image data.
	 */
	template<typename T>
	static void RotateNormal(const uint32 Width, const uint32 Height, const EInstaMATRotation Rotation, uint8* const OutBitmap)
	{
		if (OutBitmap == nullptr || Width == 0u || Height == 0u || Width == ~0u || Height == ~0u)
			return;

		if (Rotation == EInstaMATRotation::InstaMAT_Rotation0)
			return;

		/**
		 * Pixel access for data array.
		 */
		struct Pixel
		{
			T R;
			T G;
			T B;
			T A;
		};

		T InversionValue = TNumericLimits<T>::Max();
		TFunction<void(Pixel&)> Transform;

		// NOTE: float is normalized, unsigned 16 && 8 bit values are not.
		if (sizeof(T) >= 4u)
		{
			InversionValue = (T)(1);
		}

		Pixel* const Pixels = reinterpret_cast<Pixel*>(OutBitmap);

		if (Rotation == EInstaMATRotation::InstaMAT_Rotation90)
		{
			Transform = [InversionValue](Pixel& PixelValue)
			{
				PixelValue.R = InversionValue - PixelValue.R;
			};
		}
		else if (Rotation == EInstaMATRotation::InstaMAT_Rotation180)
		{
			Transform = [InversionValue](Pixel& PixelValue)
			{
				PixelValue.R = InversionValue - PixelValue.R;
				PixelValue.G = InversionValue - PixelValue.G;
			};
		}
		else if (Rotation == EInstaMATRotation::InstaMAT_Rotation270)
		{
			Transform = [InversionValue](Pixel& PixelValue)
			{
				PixelValue.G = InversionValue - PixelValue.G;
			};
		}
		else
			return;

		ParallelFor(Height, [Width, &Transform, Pixels](const uint32 Y)
		{
			const uint64 StartRow = Y * Width;
			for (uint32 X = 0u; X < Width; X++)
			{
				Pixel& PixelValue = Pixels[StartRow + X];
				Transform(PixelValue);
			}
		});
	}

	/**
	 * Applies a rotation on the specified \p OutBitmap.
	 *
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param Rotation The rotation.
	 * @param Format The texture execution format.
	 * @param [out] OutBitmap Image data.
	 */
	static void RotateNormal(const uint32 Width, const uint32 Height, const EInstaMATRotation Rotation, EInstaMATExecutionFormat Format, uint8* const OutBitmap)
	{
		switch (Format)
		{
		case EInstaMATExecutionFormat::InstaMAT_Normalized8:
			RotateNormal<uint8>(Width, Height, Rotation, OutBitmap);
			break;
		case EInstaMATExecutionFormat::InstaMAT_Normalized16:
			RotateNormal<uint16>(Width, Height, Rotation, OutBitmap);
			break;
		case EInstaMATExecutionFormat::InstaMAT_FullRange16:
			RotateNormal<uint16>(Width, Height, Rotation, OutBitmap);
			break;
		case EInstaMATExecutionFormat::InstaMAT_FullRange32:
			RotateNormal<float>(Width, Height, Rotation, OutBitmap);
			break;
		default:
			check(false);
			break;
		}
	}
}

namespace InstaMATMipmapsUtility
{
	/**
	 * Computes the size of all the mipmaps.
	 *
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param ChannelCount The image component count.
	 * @param MipsCount The number of mipmaps.
	 * @param Format The format of the data.
	 */
	static uint64 GetDataSizeFor(const uint32 Width, const uint32 Height, uint32 ChannelCount, uint32 StartMipIndex, uint32 MipsCount, EInstaMATExecutionFormat Format)
	{
		const uint64 DataBlockSize = InstaMATImageUtility::BlockSizeForExecutionFormat(Format);
		uint32 UpperMipWidth = Width;
		uint32 UpperMipHeight = Height;
		uint64 DataSize = 0u;
		for (uint32 MipIndex = StartMipIndex; MipIndex < MipsCount; MipIndex++)
		{
			const uint64 MipWidth = Width >> MipIndex;
			const uint64 MipHeight = Height >> MipIndex;
			DataSize += MipWidth * MipHeight * ChannelCount * DataBlockSize;

			if (MipWidth == 1u && MipHeight == 1u)
				break;

			UpperMipWidth = MipWidth;
			UpperMipHeight = MipHeight;
		}
		return DataSize;
	}

	namespace Internal
	{
		/**
		 * Generates a mip map values row wise for the specified \p SourceData.
		 * 
		 * @param MipRow The row to process.
		 * @param MipWidth The mipmap width.
		 * @param ChannelCount The channel count.
		 * @param SourceData The source data.
		 * @param DestinationData The destination data.
		 */
		template<typename InputType, typename SumType>
		static void ProcessRow(const uint64 MipRow, const uint32 MipWidth, const uint32 ChannelCount, const InputType* const SourceData, InputType* const DestinationData)
		{
			const uint64 RowStart = MipRow * MipWidth;
			const uint64 LowerSourceRowStart = MipRow * MipWidth * 4u;
			const uint64 UpperSourceRowStart = (MipRow * 2u + 1u) * MipWidth * 2u;

			for (uint32 X = 0u; X < MipWidth; X++)
			{
				const uint64 LowerSourceIndex = (LowerSourceRowStart + X * 2u) * ChannelCount;
				const uint64 UpperSourceIndex = (UpperSourceRowStart + X * 2u) * ChannelCount;
				const uint64 DestinationIndex = (RowStart + X) * ChannelCount;
				for (uint32 C = 0u; C < ChannelCount; C++)
				{
					SumType Sum = (SumType)SourceData[LowerSourceIndex + C] + SourceData[LowerSourceIndex + ChannelCount + C];
					Sum += (SumType)SourceData[UpperSourceIndex + C] + SourceData[UpperSourceIndex + ChannelCount + C];
					DestinationData[DestinationIndex + C] = InputType(Sum / 4u);
				}
			}
		}
	}

	/**
	 * Generates mipmaps using a box filter.
	 *
	 * @param Width The image width.
	 * @param Height The image height.
	 * @param ChannelCount The image component count.
	 * @param MipsCount The number of mipmaps.
	 * @param Format The format of the data.
	 * @param Source The source image data.
	 * @param Destination The destination mipmaps data.
	 */
	static void GenerateMipmaps(const uint32 Width, const uint32 Height, uint32 ChannelCount, uint32 MipsCount, EInstaMATExecutionFormat Format, const uint8* const Source, TArray<uint8>& Destination)
	{
		check(Destination.Num() == GetDataSizeFor(Width, Height, ChannelCount, /*StartMipIndex*/ 1u, MipsCount, Format) || Destination.Num() == 0u);
		if (Destination.IsEmpty())
		{
			Destination.SetNumUninitialized(GetDataSizeFor(Width, Height, ChannelCount, /*StartMipIndex*/ 1u, MipsCount, Format));
		}

		const uint8* SourceData = Source;
		uint8* DestinationData = Destination.GetData();

		const uint64 DataBlockSize = InstaMATImageUtility::BlockSizeForExecutionFormat(Format);
		uint32 UpperMipWidth = Width;
		uint32 UpperMipHeight = Height;
		for (uint32 MipIndex = 1u; MipIndex < MipsCount; MipIndex++)
		{
			const uint32 MipWidth = FMath::Max(Width >> MipIndex, 1u);
			const uint32 MipHeight = FMath::Max(Width >> MipIndex, 1u);
			if (Format == EInstaMATExecutionFormat::InstaMAT_Normalized8)
			{
				ParallelFor(MipHeight, [DestinationData, SourceData, MipWidth, ChannelCount](const uint32 y) {Internal::ProcessRow<uint8, uint32>(y, MipWidth, ChannelCount, SourceData, DestinationData); });
			}
			else if (Format == EInstaMATExecutionFormat::InstaMAT_Normalized16)
			{
				const uint16* SourceData16 = reinterpret_cast<const uint16*>(SourceData);
				uint16* DestinationData16 = reinterpret_cast<uint16*>(DestinationData);
				ParallelFor(MipHeight, [DestinationData16, SourceData16, MipWidth, ChannelCount](const uint32 y) {Internal::ProcessRow<uint16, uint32>(y, MipWidth, ChannelCount, SourceData16, DestinationData16); });
			}
			// TODO: support FP16
			else if (Format == EInstaMATExecutionFormat::InstaMAT_FullRange16 || Format == EInstaMATExecutionFormat::InstaMAT_FullRange32)
			{
				const float* SourceDataF = reinterpret_cast<const float*>(SourceData);
				float* DestinationDataF = reinterpret_cast<float*>(DestinationData);
				ParallelFor(MipHeight, [DestinationDataF, SourceDataF, MipWidth, ChannelCount](const uint32 y) {Internal::ProcessRow<float, float>(y, MipWidth, ChannelCount, SourceDataF, DestinationDataF); });
			}
			
			if (MipWidth == 1u && MipHeight == 1u)
				break;

			SourceData = DestinationData;
			DestinationData += MipWidth * MipHeight * ChannelCount * DataBlockSize;
			UpperMipWidth = MipWidth;
			UpperMipHeight = MipHeight;
		}
	}
}

namespace InstaMATTextureUtility
{
	/**
	 * Creates the pixel data which will be saved in the Graph instance.
	 *
	 * @param Width the texture width.
	 * @param Height the texture height.
	 * @param Data the byte data.
	 * @param DataSize the size of the data in bytes.
	 * @param Format the texture format.
	 * @return The Data pointer.
	 */
	inline static FTexturePlatformData* CreateBitmapFromData(const uint32 Width, const uint32 Height, const uint8* const Data, const uint64 DataSize, const EPixelFormat Format)
	{
		if (Width == 0u || Height == 0u)
			return new FTexturePlatformData();

		FTexturePlatformData* const Bitmap = new FTexturePlatformData();
		Bitmap->SizeX = Width;
		Bitmap->SizeY = Height;
		Bitmap->SetNumSlices(1);
		Bitmap->PixelFormat = Format;

		FTexture2DMipMap* const MipMap = new FTexture2DMipMap();
		MipMap->SizeX = Width;
		MipMap->SizeY = Height;

		Bitmap->Mips.Add(MipMap);

		MipMap->BulkData.Lock(LOCK_READ_WRITE);
		uint8* const RawPointer = static_cast<uint8*>(MipMap->BulkData.Realloc(DataSize));
		check(RawPointer != nullptr);
		FMemory::Memcpy(RawPointer, Data, DataSize);
		MipMap->BulkData.Unlock();

		return Bitmap;
	}

	/**
	 * Fills the specified \p Destination with the provided \p Data in mipmap 1.
	 * This function is used to update the texture entry on changes in the graph.
	 *
	 * @param Destination the receiving Texture object.
	 * @param Width the texture width.
	 * @param Height the texture height.
	 * @param ChannelCount the color channel component count.
	 * @param Data the byte data.
	 * @param DataSize the size of the data in bytes.
	 * @param PixelFormat The GPU pixel format.
	 * @param TextureSourceFormat The texture source format.
	 * @return True upon success.
	 */
	inline static bool FillUTexture2D(UTexture2D* const Destination, const uint32 Width, const uint32 Height, const uint32 ChannelCount, const uint8* const Data, const uint64 DataSize, EPixelFormat PixelFormat, ETextureSourceFormat TextureSourceFormat)
	{
		if (Destination == nullptr || DataSize < (Width * Height))
			return false;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 4
		Destination->ReleaseResource();
		if (Destination->GetPlatformData() != nullptr)
		{
			if (Destination->GetPlatformData()->Mips.Num() > 0)
			{
				// NOTE: Indirect array frees memory automatically 
				Destination->GetPlatformData()->Mips.Empty();
			}

			delete Destination->GetPlatformData();
			Destination->SetPlatformData(nullptr);
		}
#else
		Destination->SetPlatformData(nullptr);
		Destination->ReleaseResource();
#endif
		
		if (ChannelCount == 1u)
		{
			// grayscale always linear and no alpha channel
			Destination->CompressionNoAlpha = true;
			Destination->SRGB = false;
		}

		Destination->SetPlatformData(InstaMATTextureUtility::CreateBitmapFromData(Width, Height, Data, DataSize, PixelFormat));
		Destination->Source.Init(Width, Height, 1, 1, TextureSourceFormat, Data);
		Destination->UpdateResource();
		return true;
	}

	/**
	 * Gets the texture compression format based on the specified input parameters.
	 *
	 * @param TextureName the Texture name.
	 * @param ChannelCount the channel count.
	 * @param Format the execution format.
	 * @return The compression setting.
	 */
	inline static TextureCompressionSettings GetTextureCompressionFormat(const FString& TextureName, const uint32 ChannelCount, const EInstaMATExecutionFormat Format)
	{
		if (Format == EInstaMATExecutionFormat::InstaMAT_FullRange32 && ChannelCount == 1)
			return TextureCompressionSettings::TC_SingleFloat;

		if (TextureName.Contains(TEXT("normal"), ESearchCase::IgnoreCase))
			return TextureCompressionSettings::TC_Normalmap;

		if (TextureName.Contains(TEXT("mask"), ESearchCase::IgnoreCase))
			return TextureCompressionSettings::TC_Masks;

		if (Format == EInstaMATExecutionFormat::InstaMAT_FullRange16)
		{
			// TODO: add compression settings for FR16 execution.
		}

		if (Format == EInstaMATExecutionFormat::InstaMAT_FullRange32 && ChannelCount >= 3)
			return TextureCompressionSettings::TC_HDR_F32;

		if (ChannelCount >= 3)
			return TextureCompressionSettings::TC_Default;

		if (ChannelCount == 1)
			return TextureCompressionSettings::TC_Grayscale;
		
		return TextureCompressionSettings::TC_Default;
	}

	/**
	 * Fills the specified \p Destination with the provided \p Data in mipmap 1.
	 * This function is used to update the texture entry on changes in the graph.
	 *
	 * @param Destination the receiving Texture object.
	 * @param Width the texture width.
	 * @param Height the texture height.
	 * @param ChannelCount the color channel component count.
	 * @param Data the byte data.
	 * @param DataSize the size of the data in bytes.
	 * @param Format The graph execution format.
	 * @return True upon success.
	 */
	inline static bool FillUTexture2D(UTexture2D* const Destination, const uint32 Width, const uint32 Height, const uint32 ChannelCount, const uint8* const Data, const uint64 DataSize, const EInstaMATExecutionFormat Format)
	{
		check(ChannelCount >= 1u && ChannelCount <= 4u);
		if (Destination == nullptr || DataSize < (Width * Height * ChannelCount))
			return false;

		const EPixelFormat PixelFormat = InstaMATImageUtility::PixelFormatFromExecutionFormat(Format, ChannelCount);
		const ETextureSourceFormat TextureSourceFormat = InstaMATImageUtility::TextureSourceFormatFromExecutionFormat(Format, ChannelCount);
		TArray<uint8> ConvertedData;

		switch (Format)
		{
		case EInstaMATExecutionFormat::InstaMAT_Normalized8:
			
			if (ChannelCount == 1u)
				break;

			InstaMATImageUtility::ConvertFromRGBAToBGRA8(Width, Height, ChannelCount, /*SourceBlockBytes*/ 1u, Data, /*Out*/ ConvertedData);
			break;

		case EInstaMATExecutionFormat::InstaMAT_Normalized16:

			if (ChannelCount == 1u)
				break;
		
			InstaMATImageUtility::ConvertToRGBA_N16(Width, Height, ChannelCount, Data, /*out*/ ConvertedData);
			break;

		case EInstaMATExecutionFormat::InstaMAT_FullRange16:
			//TODO: handle F16

		case EInstaMATExecutionFormat::InstaMAT_FullRange32:

			if (ChannelCount == 1u)
				break;

			InstaMATImageUtility::ConvertToRGBA_F32(Width, Height, ChannelCount, Data, /*out*/ ConvertedData);
			break;

		default:
			return false;
		}

		const uint8* SourceData = ConvertedData.IsEmpty() ? Data : ConvertedData.GetData();
		const uint64 SourceDataSize = ConvertedData.IsEmpty() ? DataSize : ConvertedData.Num();
		FillUTexture2D(Destination, Width, Height, ChannelCount, SourceData, SourceDataSize, PixelFormat, TextureSourceFormat);

		return true;
	}

	/**
	 * Fills the specified \p Destination with the provided \p DataSource in mipmap 1.
	 * This function is used to update the texture entry on changes in the graph.
	 *
	 * @param Destination the receiving Texture object.
	 * @param DataSource The texture data source, which can be from a sampler or raw data.
	 * @return True upon success.
	 */
	inline static bool FillUTexture2D(UTexture2D* const Destination, const FInstaMATTextureDataSource& DataSource)
	{
		if (Destination == nullptr)
			return false;

		const EPixelFormat SourcePixelFormat = DataSource.GetDataPixelFormat();
		if (SourcePixelFormat == EPixelFormat::PF_B8G8R8A8)
		{
			ETextureSourceFormat TextureSourceFormat = ETextureSourceFormat::TSF_BGRA8;
			// no conversion needed
			return FillUTexture2D(Destination, DataSource.GetWidth(), DataSource.GetHeight(), DataSource.GetChannelCount(), DataSource.GetData(), DataSource.GetDataSize(), SourcePixelFormat, TextureSourceFormat);
		}

		const EInstaMATExecutionFormat ExecutionFormat = InstaMATImageUtility::ExecutionFormatFromPixelFormat(SourcePixelFormat);
		return FillUTexture2D(Destination, DataSource.GetWidth(), DataSource.GetHeight(), DataSource.GetChannelCount(), DataSource.GetData(), DataSource.GetDataSize(), ExecutionFormat);
	}

	/**
	 * Returns the texture data as an byte array from the specified \p Texture.
	 *
	 * @param Texture the texture.
	 * @return The texture data.
	 */
	inline static TArray<uint8> GetTextureDataFromTexture2D(UTexture2D* const Texture)
	{
		check(Texture != nullptr);

		TArray<uint8> Data;
		if (!Texture->Source.IsValid())
		{
			UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Could not retrieve texture data, Texture Source is Invalid."));
			return Data;
		}

#if defined (WITH_EDITOR)
		const uint32 Width = Texture->Source.GetSizeX();
		const uint32 Height = Texture->Source.GetSizeY();
		const ETextureSourceFormat Format = Texture->Source.GetFormat();
		const uint32 PixelCount = Width * Height;

		/// The fnFillDataArrayFromLByte Lambda converts single channel byte into rgba byte format
		const auto fnFillDataArrayFromLByte = [&](const uint8* const Values)
		{
			constexpr uint8 kUInt8Max = TNumericLimits<uint8>::Max();

			for (uint32 PixelIndex = 0u; PixelIndex < PixelCount; PixelIndex++)
			{
				const uint32 DataIndex = PixelIndex * 4u;
				Data[DataIndex] = Values[PixelIndex];		// R
				Data[DataIndex + 1u] = Values[PixelIndex];	// G
				Data[DataIndex + 2u] = Values[PixelIndex];	// B
				Data[DataIndex + 3u] = kUInt8Max;			// A
			}
		};
		/// The fnFillDataArrayFromLByte Lambda converts single channel short into rgba byte format
		const auto fnFillDataArrayFromLShort = [&](const uint8* const Values)
		{
			constexpr uint8 kUInt8Max = TNumericLimits<uint8>::Max();
			constexpr uint16 kUInt16Max = TNumericLimits<uint16>::Max();

			for (uint32 PixelIndex = 0u; PixelIndex < PixelCount; PixelIndex++)
			{
				const uint32 DataIndex = PixelIndex * 4u;
				const uint16* const C = reinterpret_cast<const uint16*>(&Values[PixelIndex * sizeof(uint16)]);
				const uint8 Value = (*C / (float)kUInt16Max) * kUInt8Max;

				Data[DataIndex] = Value;		// R
				Data[DataIndex + 1u] = Value;	// G
				Data[DataIndex + 2u] = Value;	// B
				Data[DataIndex + 3u] = Value;	// A
			}
		};
		/// The fnFillDataArrayFromLByte Lambda converts single channel half float into rgba byte format
		const auto fnFillDataArrayFromLHalfFloat = [&](const uint8* const Values)
		{
			const FFloat16 kFloat16Max = FFloat16::MaxF16Float;
			constexpr uint8 kUInt8Max = TNumericLimits<uint8>::Max();

			for (uint32 PixelIndex = 0u; PixelIndex < PixelCount; PixelIndex++)
			{
				const uint32 DataIndex = PixelIndex * 4u;
				const FFloat16* const C = reinterpret_cast<const FFloat16*>(&Values[PixelIndex * sizeof(FFloat16)]);
				const uint8 Value = FMath::Clamp(C->GetFloat(), 0.0f, 1.0f) * kUInt8Max;

				Data[DataIndex] = Value;		// R
				Data[DataIndex + 1u] = Value;	// G
				Data[DataIndex + 2u] = Value;	// B
				Data[DataIndex + 3u] = Value;	// A
			}
		};
		/// The fnFillDataArrayFromLByte Lambda converts single channel float into rgba byte format
		const auto fnFillDataArrayFromLFloat = [&](const uint8* const Values)
		{
			constexpr uint8 kUInt8Max = TNumericLimits<uint8>::Max();

			for (uint32 PixelIndex = 0u; PixelIndex < PixelCount; PixelIndex++)
			{
				const uint32 DataIndex = PixelIndex * 4u;
				const float* const C = reinterpret_cast<const float*>(&Values[PixelIndex * sizeof(float)]);
				const uint8 Value = FMath::Clamp(*C, 0.0f, 1.0f) * kUInt8Max;

				Data[DataIndex] = Value;		// R
				Data[DataIndex + 1u] = Value;	// G
				Data[DataIndex + 2u] = Value;	// B
				Data[DataIndex + 3u] = Value;	// A
			}
		};
		/// The fnFillDataArrayFromLByte Lambda converts bgra byte into rgba byte format
		const auto fnFillDataArrayFromBGRAByte = [&](const uint8* const Values)
		{
			for (uint32 PixelIndex = 0u; PixelIndex < PixelCount; PixelIndex++)
			{
				const uint32 DataIndex = PixelIndex * 4u;
				FColor Color(Values[DataIndex + 2u], Values[DataIndex + 1u], Values[DataIndex], Values[DataIndex + 3u]);

				// NOTE: it seems the colors are in sRGB color space, we need to convert them down.
				const FLinearColor Intermediate = FLinearColor(Color);
				Color = Intermediate.ToFColor(false);

				Data[DataIndex] = Color.R;		// R
				Data[DataIndex + 1u] = Color.G;	// G
				Data[DataIndex + 2u] = Color.B;	// B
				Data[DataIndex + 3u] = Color.A;	// A
			}
		};
		/// The fnFillDataArrayFromLByte Lambda converts rgba short into rgba byte format
		const auto fnFillDataArrayFromRGBAShort = [&](const uint8* const Values)
		{
			constexpr uint8 kUInt8Max = TNumericLimits<uint8>::Max();
			constexpr uint16 kUInt16Max = TNumericLimits<uint16>::Max();

			for (uint32 PixelIndex = 0u; PixelIndex < PixelCount; PixelIndex++)
			{
				const uint32 DataIndex = PixelIndex * 4u;
				const uint32 SourceDataIndex = PixelIndex * 4u * sizeof(uint16);
				const uint16* const R = reinterpret_cast<const uint16*>(&Values[SourceDataIndex]);
				const uint16* const G = reinterpret_cast<const uint16*>(&Values[SourceDataIndex + sizeof(uint16)]);
				const uint16* const B = reinterpret_cast<const uint16*>(&Values[SourceDataIndex + sizeof(uint16) * 2u]);
				const uint16* const A = reinterpret_cast<const uint16*>(&Values[SourceDataIndex + sizeof(uint16) * 3u]);

				Data[DataIndex] =		FMath::Clamp((*R / (float)kUInt16Max) * kUInt8Max, 0, kUInt8Max);			// R
				Data[DataIndex + 1u] =	FMath::Clamp((*G / (float)kUInt16Max) * kUInt8Max, 0, kUInt8Max);	// G
				Data[DataIndex + 2u] =	FMath::Clamp((*B / (float)kUInt16Max) * kUInt8Max, 0, kUInt8Max);	// B
				Data[DataIndex + 3u] =	FMath::Clamp((*A / (float)kUInt16Max) * kUInt8Max, 0, kUInt8Max);	// A
			}
		};
		/// The fnFillDataArrayFromLByte Lambda converts rgba half float into rgba byte format
		const auto fnFillDataArrayFromRGBAHalfFloat = [&](const uint8* const Values)
		{
			const FFloat16 kFloat16Max = FFloat16::MaxF16Float;
			constexpr uint8 kUInt8Max = TNumericLimits<uint8>::Max();

			for (uint32 PixelIndex = 0u; PixelIndex < PixelCount; PixelIndex++)
			{
				const uint32 DataIndex = PixelIndex * 4u;
				const FFloat16Color* const Color = reinterpret_cast<const FFloat16Color*>(&Values[PixelIndex * sizeof(FFloat16Color)]);
				const FLinearColor LinearColor = Color->GetFloats();

				Data[DataIndex] =		FMath::Clamp(LinearColor.R, 0.0f, 1.0f) * kUInt8Max;		// R
				Data[DataIndex + 1u] =	FMath::Clamp(LinearColor.G, 0.0f, 1.0f) * kUInt8Max;	// G
				Data[DataIndex + 2u] =	FMath::Clamp(LinearColor.B, 0.0f, 1.0f) * kUInt8Max;	// B
				Data[DataIndex + 3u] =	FMath::Clamp(LinearColor.A, 0.0f, 1.0f) * kUInt8Max;	// A
			}
		};
		/// The fnFillDataArrayFromLByte Lambda converts rgba float into rgba byte format
		const auto fnFillDataArrayFromRGBAFloat = [&](const uint8* const Values)
		{
			constexpr uint8 kUInt8Max = TNumericLimits<uint8>::Max();

			for (uint32 PixelIndex = 0u; PixelIndex < PixelCount; PixelIndex++)
			{
				const uint32 DataIndex = PixelIndex * 4u;
				const uint32 SourceDataIndex = PixelIndex * 4u * sizeof(float);
				const float* const R = reinterpret_cast<const float*>(&Values[SourceDataIndex]);
				const float* const G = reinterpret_cast<const float*>(&Values[SourceDataIndex + sizeof(float)]);
				const float* const B = reinterpret_cast<const float*>(&Values[SourceDataIndex + sizeof(float) * 2u]);
				const float* const A = reinterpret_cast<const float*>(&Values[SourceDataIndex + sizeof(float) * 3u]);

				Data[DataIndex] =		FMath::Clamp(*R, 0.0f, 1.0f) * kUInt8Max;			// R
				Data[DataIndex + 1u] =	FMath::Clamp(*G, 0.0f, 1.0f) * kUInt8Max;	// G
				Data[DataIndex + 2u] =	FMath::Clamp(*B, 0.0f, 1.0f) * kUInt8Max;	// B
				Data[DataIndex + 3u] =	FMath::Clamp(*A, 0.0f, 1.0f) * kUInt8Max;	// A
			}
		};

		const uint8* const DataSource = Texture->Source.LockMipReadOnly(0);

		if (DataSource != nullptr)
		{
			// NOTE: the output array shall be in the format R8G8B8A8 
			Data.SetNumZeroed(PixelCount * 4u * 1u);

			switch (Format)
			{
			case ETextureSourceFormat::TSF_G8:
				fnFillDataArrayFromLByte(DataSource);
				break;
			case ETextureSourceFormat::TSF_BGRA8:
			case ETextureSourceFormat::TSF_BGRE8:
				fnFillDataArrayFromBGRAByte(DataSource);
				break;
			case ETextureSourceFormat::TSF_RGBA16:
				fnFillDataArrayFromRGBAShort(DataSource);
				break;
			case ETextureSourceFormat::TSF_RGBA16F:
				fnFillDataArrayFromRGBAHalfFloat(DataSource);
				break;
			case ETextureSourceFormat::TSF_G16:
				fnFillDataArrayFromLShort(DataSource);
				break;
			case ETextureSourceFormat::TSF_RGBA32F:
				fnFillDataArrayFromRGBAFloat(DataSource);
				break;
			case ETextureSourceFormat::TSF_R16F:
				fnFillDataArrayFromLHalfFloat(DataSource);
				break;
			case ETextureSourceFormat::TSF_R32F:
				fnFillDataArrayFromLFloat(DataSource);
				break;
			default:
				UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Texture Format is not supported."));
				Data.SetNum(0u, /*bAllowShrinking:*/ true);
				break;
			}
		}

		Texture->Source.UnlockMip(0);

#else
		if (Texture->GetPlatformData() == nullptr)
			return TArray<uint8>();

		// NOTE: Need to update compression to avoid nullptr texture data, source: https://archive.is/nQIgR
		TextureCompressionSettings MipMapSettings = Texture->CompressionSettings;
		TextureMipGenSettings MipGenSettings = Texture->MipGenSettings;
		bool bIsSRGB = Texture->SRGB;

		Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		Texture->SRGB = false;
		Texture->UpdateResource();

		/// The fnRevertTextureSettings lambda reverts to the default settings on invocation.
		const auto fnRevertTextureSettings = [MipMapSettings, MipGenSettings, bIsSRGB, Texture]()
		{
			Texture->CompressionSettings = MipMapSettings;
			Texture->MipGenSettings = MipGenSettings;
			Texture->SRGB = bIsSRGB;
			Texture->UpdateResource();
		};

		const FColor* const TextureData = static_cast<const FColor*>(Texture->GetPlatformData()->Mips[0].BulkData.LockReadOnly());

		if (TextureData == nullptr)
		{
			Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
			fnRevertTextureSettings();
			return TArray<uint8>();
		}
		const uint64 TextureDataSize = Texture->Source.GetSizeX() * Texture->Source.GetSizeY() * 4u;
		Data.AddUninitialized(TextureDataSize);

		for (uint32 TargetIndex = 0u, SourceIndex = 0u; TargetIndex < TextureDataSize; TargetIndex += 4, SourceIndex++)
		{
			Data[TargetIndex] = TextureData[SourceIndex].R;
			Data[TargetIndex + 1u] = TextureData[SourceIndex].G;
			Data[TargetIndex + 2u] = TextureData[SourceIndex].B;
			Data[TargetIndex + 3u] = TextureData[SourceIndex].A;
		}

		Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
		fnRevertTextureSettings();
#endif

		return Data;
	}

	/**
	 * Creates a FColor array of the provided unsigned \p Data object.
	 *
	 * @param TextureWidth the texture width
	 * @param TextureHeight the texture height
	 * @param ColorChannelCount the channel per pixel count
	 * @param Data the data
	 * @return Array of FColor
	 */
	static TArray<FColor> CreateColorSamplesFromByteData(const uint32 TextureWidth, const uint32 TextureHeight, const uint32 ColorChannelCount, const TArray<uint8>& Data)
	{
		check(ColorChannelCount < 5u);

		/// The fnCreateSamplesFromRGBA lambda creates a samples array from the specified four channel data
		const auto fnCreateSamplesFromRGBA = [TextureWidth, TextureHeight](const TArray<uint8>& Values) -> TArray<FColor>
		{
			TArray<FColor> Samples;
			const uint32 PixelCount = TextureHeight * TextureWidth;
			Samples.SetNumZeroed(PixelCount);
			for (uint32 Index = 0u; Index < PixelCount; Index++)
			{
				const uint32 SourcePixelIndex = Index * 4u;
				Samples[Index] = FColor(Values[SourcePixelIndex], Values[SourcePixelIndex + 1u], Values[SourcePixelIndex + 2u], Values[SourcePixelIndex + 3u]);
			}
			return Samples;
		};

		/// The fnCreateSamplesFromLuminance lambda creates a samples array from the specified single channel data
		const auto fnCreateSamplesFromLuminance = [TextureWidth, TextureHeight](const TArray<uint8>& Values) -> TArray<FColor>
		{
			TArray<FColor> Samples;
			const uint32 PixelCount = TextureHeight * TextureWidth;
			Samples.SetNumZeroed(PixelCount);
			for (uint32 Index = 0u; Index < PixelCount; Index++)
			{
				Samples[Index] = FColor(Values[Index], 0u, 0u, TNumericLimits<uint8>::Max());
			}
			return Samples;
		};

		/// The fnCreateSamplesFromRGB lambda creates a samples array from the specified three channel data
		const auto fnCreateSamplesFromRGB = [TextureWidth, TextureHeight](const TArray<uint8>& Values) -> TArray<FColor>
		{
			TArray<FColor> Samples;
			const uint32 PixelCount = TextureHeight * TextureWidth;
			Samples.SetNumZeroed(PixelCount);
			for (uint32 Index = 0u; Index < PixelCount; Index++)
			{
				const uint32 SourcePixelIndex = Index * 3u;
				Samples[Index] = FColor(Values[SourcePixelIndex], Values[SourcePixelIndex + 1u], Values[SourcePixelIndex + 2u], TNumericLimits<uint8>::Max());
			}
			return Samples;
		};

		/// The fnCreateSamplesFromLuminanceAlpha lambda creates a samples array from the specified two channel data
		const auto fnCreateSamplesFromLuminanceAlpha = [TextureWidth, TextureHeight](const TArray<uint8>& Values) -> TArray<FColor>
		{
			TArray<FColor> Samples;
			const uint32 PixelCount = TextureHeight * TextureWidth;
			Samples.SetNumZeroed(PixelCount);
			for (uint32 Index = 0u; Index < PixelCount; Index++)
			{
				const uint32 SourcePixelIndex = Index * 2u;
				Samples[Index] = FColor(Values[SourcePixelIndex], 0u, 0u, Values[SourcePixelIndex + 1u]);
			}
			return Samples;
		};

		if (ColorChannelCount == 1u)
			return fnCreateSamplesFromLuminance(Data);

		if (ColorChannelCount == 2u)
			return fnCreateSamplesFromLuminanceAlpha(Data);

		if (ColorChannelCount == 3u)
			return fnCreateSamplesFromRGB(Data);

		return fnCreateSamplesFromRGBA(Data);
	}
}
