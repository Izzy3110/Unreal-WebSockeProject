// Copyright Athian Games. All Rights Reserved. 


#include "MySQLBPLibrary.h"

#include <string>

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "TextureResource.h"
#include "RenderingThread.h"
#include "Templates/SharedPointer.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"

IImageWrapperModule* ImageWrapperModule = nullptr;

void UMySQLBPLibrary::GetTexturePixels(UTexture2D* Texture, TArray<FColor>& OutPixels)
{
	if (!Texture) return;

	OutPixels.Reset();

	ENQUEUE_RENDER_COMMAND(FSaveMySQLImageResource)(
		[Texture, &OutPixels](FRHICommandListImmediate& RHICmdList)
		{
			FTextureResource* TextureResource = Texture->GetResource();
			if (!TextureResource || !TextureResource->TextureRHI.IsValid())
				return;

			// ✅ Use unified FTextureRHIRef instead of deprecated FTexture2DRHIRef
			FTextureRHIRef TextureRHI = TextureResource->TextureRHI;
			FIntPoint Size = Texture->GetImportedSize();

			TArray<FColor> TempData;
			FIntRect Rect(0, 0, Size.X, Size.Y);
			FReadSurfaceDataFlags Flags(RCM_UNorm, CubeFace_MAX);

			// Read pixels from the texture surface
			RHICmdList.ReadSurfaceData(TextureRHI, Rect, TempData, Flags);

			// Copy data back to OutPixels (on render thread)
			OutPixels = MoveTemp(TempData);
		});
}



UTexture2D* UMySQLBPLibrary::LoadTexturefromCharData(char* ImageChar)
{
	CreateImageWrapperModule();

	if (ImageWrapperModule)
	{
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule->CreateImageWrapper(EImageFormat::PNG);

		int length = reinterpret_cast<size_t>(ImageChar);
		TArray<uint8> CompressedImageData;
		CompressedImageData.AddUninitialized(length);

		for (int i = 0; i < length; i++)
		{
			CompressedImageData[i] = ImageChar[i];
		}

		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(CompressedImageData.GetData(),
		                                                          CompressedImageData.Num()))
		{
			TArray<uint8> UncompressedBGRA;
			if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
			{
				if (UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(),
				                                                      ImageWrapper->GetHeight(), PF_B8G8R8A8))
				{
					void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
					FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
					Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

					Texture->UpdateResource();
					return Texture;
				}
			}
		}
	}

	return nullptr;
}

char* UMySQLBPLibrary::GetCharFromTextureData(UTexture2D* Texture, FString Path)
{
	FVector2D ImageSize = FVector2D(Texture->GetSurfaceWidth(), Texture->GetSurfaceHeight());
	char* imagebytes = new char[ImageSize.X * ImageSize.Y * 4];


	if (Texture->GetPlatformData()->Mips.Num() > 0)
	{
		TextureCompressionSettings OldCompressionSettings = Texture->CompressionSettings;
#if WITH_EDITORONLY_DATA
		TextureMipGenSettings OldMipGenSettings = Texture->MipGenSettings;
#endif
		bool OldSRGB = Texture->SRGB;

		Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
#if WITH_EDITORONLY_DATA
		Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif

		Texture->SRGB = false;
		Texture->UpdateResource();

		FTexture2DMipMap* Mip = &Texture->GetPlatformData()->Mips[0];
		uint8* TextureData = static_cast<uint8*>(Mip->BulkData.Lock(LOCK_READ_WRITE));


		for (int i = 0; i < (ImageSize.X * ImageSize.Y); i += 4)
		{
			imagebytes[i] = TextureData[i];
			imagebytes[i + 1] = TextureData[i + 1];
			imagebytes[i + 2] = TextureData[i + 2];
			imagebytes[i + 3] = TextureData[i + 3];
		}


		TArray<FColor> OutPixels;
		GetTexturePixels(Texture, OutPixels);


		Texture->CompressionSettings = OldCompressionSettings;
#if WITH_EDITORONLY_DATA
		Texture->MipGenSettings = OldMipGenSettings;
#endif
		Texture->SRGB = OldSRGB;

		Mip->BulkData.Unlock();
		Texture->UpdateResource();
	}

	return imagebytes;
}


bool UMySQLBPLibrary::SaveTextureToPath(UTexture2D* Texture, const FString Path)
{
	if (Texture && FPaths::ValidatePath(Path))
	{
		FVector2D ImageSize = FVector2D(Texture->GetSurfaceWidth(), Texture->GetSurfaceHeight());

		if (char* imagebytes = GetCharFromTextureData(Texture, Path))
		{
			char* path = TCHAR_TO_ANSI(*Path);
			return false;
		}
	}
	return false;
}


void UMySQLBPLibrary::CreateImageWrapperModule()
{
	if (!ImageWrapperModule)
	{
		ImageWrapperModule = &FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	}
}

char* UMySQLBPLibrary::GetRawImageFromPath(FString ImagePath)
{
	CreateImageWrapperModule();
	void* imagebytes = nullptr;
	if (ImageWrapperModule)
	{
		FString Extension = FPaths::GetExtension(ImagePath);
		EImageFormat ImageFormat;

		if (Extension.Equals("jpeg", ESearchCase::IgnoreCase) || Extension.Equals("jpg", ESearchCase::IgnoreCase))
		{
			ImageFormat = EImageFormat::JPEG;
		}
		else if (Extension.Equals("png", ESearchCase::IgnoreCase))
		{
			ImageFormat = EImageFormat::PNG;
		}
		else
		{
			ImageFormat = EImageFormat::BMP;
		}

		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule->CreateImageWrapper(ImageFormat);

		TArray<uint8> RawFileData;
		if (FFileHelper::LoadFileToArray(RawFileData, *ImagePath))
		{
			if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
			{
				TArray<uint8> UncompressedBGRA;
				if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
				{
					const int64 RawNum = UncompressedBGRA.Num();
					imagebytes = FMemory::Malloc(RawNum);
					FMemory::Memcpy(imagebytes, UncompressedBGRA.GetData(), RawNum);
				}
			}
		}
	}

	return static_cast<char*>(imagebytes);
}


wchar_t* UMySQLBPLibrary::GetWCharfromChar(const char* Input)
{
	// Convert UTF-8 encoded char* to FString
	FString InputFString(UTF8_TO_TCHAR(Input));

	// Get the length of the FString (including null-terminator)
	int32 length = InputFString.Len() + 1;

	// Allocate memory for the wchar_t* buffer
	wchar_t* wcsText = new wchar_t[length];

	// Copy the contents of the FString to the wchar_t* buffer
	memcpy(wcsText, *InputFString, length * sizeof(wchar_t));

	return wcsText;
}

char* UMySQLBPLibrary::GetCharfromFString(FString Query)
{
	// Convert FString to UTF-8
	std::string QueryUTF8(TCHAR_TO_UTF8(*Query));

	// Allocate memory for the char buffer
	char* charBuffer = static_cast<char*>(malloc(QueryUTF8.size() + 1));

	// Copy the contents of the UTF-8 string to the char buffer
	memcpy(charBuffer, QueryUTF8.c_str(), QueryUTF8.size() + 1);

	return charBuffer;
}

TArray<FString> UMySQLBPLibrary::GetSplitStringArray(FString Input, FString Pattern)
{
	TArray<FString> Results;

	FString Base;
	FString Left;
	FString Remaining;

	if (Input.Split(Pattern, &Base, &Remaining))
	{
		Results.Add(Base);

		while (Remaining != "")
		{
			if (Remaining.Split(Pattern, &Left, &Remaining))
			{
				Results.Add(Left);
			}
			else
			{
				Results.Add(Remaining);
				Remaining = "";
			}
		}
	}
	else
	{
		Results.Add(Input);
	}

	return Results;
}
