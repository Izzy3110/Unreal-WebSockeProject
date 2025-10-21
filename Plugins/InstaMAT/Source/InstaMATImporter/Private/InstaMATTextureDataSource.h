/**
 * InstaMATTextureDataSource.h (InstaMAT)
 *
 * Copyright 2019-2023 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATTextureDataSource.h
 * @copyright 2019-2023 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

/**
 * The FInstaMATTextureDataSource holds the streaming data for output textures.
 */
class FInstaMATTextureDataSource
{
public:
	/**
	 * Imports the Output Texture Parameters for the specified object.
	 *
	 * @param Name the name of the texture to update.
	 * @param Interface the InstaMAT API Interface.
	 * @param ImageSampler the sampler from which to get the data from.
	 * @param ImageFormat the pixel format of the sampler.
	 */
	FInstaMATTextureDataSource(const FString& Name, IInstaMAT* const Interface, InstaMAT::IImageSampler* const ImageSampler, const EPixelFormat ImageFormat) :
	Name(Name),
	InstaMATInterface(Interface),
	Sampler(ImageSampler),
	SamplerFormat(ImageFormat),
	Width(ImageSampler->GetWidth()),
	Height(ImageSampler->GetHeight()),
	ChannelCount(ImageSampler->GetComponentCount()),
	NumMipmaps(0u)
	{
		check(ImageSampler != nullptr);
	}

	~FInstaMATTextureDataSource()
	{
		if (Sampler != nullptr)
		{
			InstaMATInterface->GetInstaMAT()->DeallocImageSampler(Sampler);
		}
	}

	/**
	 * Sets the converted data of this instance.
	 *
	 * @param Data the pixel data.
	 * @param DataPixelFormat the pixel format of the data.
	 * @param bFreeSampler Whether the sampler should be freed.
	 */
	inline void SetConvertedData(const TSharedPtr<TArray<uint8>>& Data, const EPixelFormat DataPixelFormat, bool bFreeSampler)
	{
		check(Data.IsValid());

		ConvertedData = Data;
		ConvertedFormat = DataPixelFormat;

		if (bFreeSampler && Sampler != nullptr)
		{
			InstaMATInterface->GetInstaMAT()->DeallocImageSampler(Sampler);
			Sampler = nullptr;
		}
	}

	/**
	 * Gets the texture raw data.
	 *
	 * @param bPreferSamplerOverConverted if enabled then it will choose the sampler data over the converted data if both are present.
	 * @return the raw texture data.
	 */
	inline const uint8* GetData(bool bPreferSamplerOverConverted = true) const
	{
		if (CheckPreferSamplerOverConverted(bPreferSamplerOverConverted))
			return Sampler->GetData(nullptr);

		return ConvertedData->GetData();
	}

	/**
	 * Gets the pixel data size.
	 * 
	 * @param bPreferSamplerOverConverted Whether the data should be retrieved from the sampler or data storage.
	 * @return The data size.
	 */
	inline uint64 GetDataSize(bool bPreferSamplerOverConverted = true) const
	{
		if (CheckPreferSamplerOverConverted(bPreferSamplerOverConverted))
		{
			uint64 DataSize = 0u;
			Sampler->GetData(&DataSize);
			return DataSize;
		}

		return ConvertedData->Num();
	}

	/**
	 * Sets the mipmaps data, used for GPU updates.
	 *
	 * @param Data the raw data.
	 * @param NumMips the number of mipmaps.
	 */
	FORCEINLINE void SetMipmapsData(const TSharedPtr<TArray<uint8>>& Data, const uint32 NumMips)
	{
		check(Data.IsValid());

		MipmapsData = Data;
		NumMipmaps = NumMips;
	}

	/**
	 * Gets the mipmaps raw data.
	 */
	FORCEINLINE const uint8* GetMipmapsData() const
	{
		return MipmapsData->GetData();
	}

	/**
	* Gets the width of the bitmap.
	* 
	* @return The width.
	*/
	FORCEINLINE uint32 GetWidth() const
	{
		return Width;
	}

	/**
	 * Gets the height of the bitmap.
	 *
	 * @return The height.
	 */
	FORCEINLINE uint32 GetHeight() const
	{
		return Height;
	}

	/**
	 * Gets the channel count of the bitmap.
	 *
	 * @return The channel count.
	 */
	FORCEINLINE uint32 GetChannelCount() const
	{
		return ChannelCount;
	}

	/**
	* Returns the number of mipmaps.
	* 
	* @return The number of mip maps.
	*/
	FORCEINLINE uint32 GetNumMipmaps() const
	{
		return NumMipmaps;
	}

	/**
	 * Gets the texture's pixel format.
	 * @note The pixel format of the sampler and of the converted data can be different.
	 *
	 * @param bPreferSamplerOverConverted if enabled then it will choose the sampler data over the converted data if both are present.
	 * @return the texture's pixel format.
	 */
	inline EPixelFormat GetDataPixelFormat(bool bPreferSamplerOverConverted = true) const
	{
		if (CheckPreferSamplerOverConverted(bPreferSamplerOverConverted))
			return SamplerFormat;

		return ConvertedFormat;
	}

private:

	/**
	 * Determines whether the sampler data should be used.
	 *
	 * @param bPreferSampler Whether the sampler should be preferred.
	 * @return True if sampler is valid and the parameter preferres the sampler.
	 */
	inline bool CheckPreferSamplerOverConverted(bool bPreferSampler) const 
	{
		return (bPreferSampler && Sampler != nullptr) || !ConvertedData.IsValid();
	}

	FInstaMATTextureDataSource(const FInstaMATTextureDataSource&) = delete;
	FInstaMATTextureDataSource& operator=(FInstaMATTextureDataSource const&) = delete;

	FString Name;								/**< The Name of this instance. */
	IInstaMAT* const InstaMATInterface;			/**< The InstaMAT interface. */
	InstaMAT::IImageSampler* Sampler;			/**< The image sampler. */
	EPixelFormat SamplerFormat;					/**< The pixel format of the sampler. */
	TSharedPtr<TArray<uint8>> ConvertedData;	/**< The data storage. */
	TSharedPtr<TArray<uint8>> MipmapsData;		/**< The mipmaps data storage. */
	EPixelFormat ConvertedFormat;				/**< The converted data storage bitmap format. */
	uint32 Width;								/**< The bitmap width. */
	uint32 Height;								/**< The bitmap height. */
	uint32 ChannelCount;						/**< The bitmap channel count. */
	uint32 NumMipmaps;							/**< The mipmaps count. */
};
