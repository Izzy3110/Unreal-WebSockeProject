/**
 * InstaMATPluginStyle.cpp (InstaMAT)
 *
 * Copyright 2016-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all its contents are proprietary and confidential.
 */

#include "InstaMATPCH.h"
#include "Slate/InstaMATPluginStyle.h"

#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

// === Helper Macros ===
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...)   FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...)    FSlateFontInfo(Style->RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)

TSharedPtr<FSlateStyleSet> FInstaMATPluginStyle::StyleInstance;

void FInstaMATPluginStyle::Initialize()
{
	if (StyleInstance.IsValid())
	{
		return;
	}

	// Create style set
	const FString StyleSetName = TEXT("InstaMATPluginStyle");
	TSharedRef<FSlateStyleSet> Style = MakeShared<FSlateStyleSet>(FName(*StyleSetName));

	// Point content root to <PluginDir>/Resources
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("InstaMAT"));
	if (Plugin.IsValid())
	{
		Style->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
	}
	else
	{
		Style->SetContentRoot(FPaths::ProjectContentDir() / TEXT("InstaMAT/Resources"));
	}

	// === Image definitions ===
	const FVector2D Icon40(40.f, 40.f);
	const FVector2D Icon20(20.f, 20.f);

	Style->Set(TEXT("InstaMAT.OpenPluginWindow"), new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40));
	Style->Set(TEXT("InstaMAT.SmallIcon"),       new IMAGE_BRUSH(TEXT("ButtonIcon_20x"), Icon20));
	Style->Set(TEXT("InstaMAT.Logo"),            new IMAGE_BRUSH(TEXT("logo"), FVector2D(256.f, 64.f)));

	// === Font definitions ===
	Style->Set(TEXT("InstaMAT.HeaderFont"), TTF_FONT(TEXT("Fonts/Roboto-Bold"), 16));
	Style->Set(TEXT("InstaMAT.BodyFont"),   TTF_FONT(TEXT("Fonts/Roboto-Regular"), 10));

	// Register the style
	FSlateStyleRegistry::RegisterSlateStyle(*Style);
	StyleInstance = Style;
}

const ISlateStyle& FInstaMATPluginStyle::Get()
{
	return *StyleInstance;
}

FName FInstaMATPluginStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("InstaMATPluginStyle"));
	return StyleSetName;
}

void FInstaMATPluginStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

void FInstaMATPluginStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef TTF_FONT
