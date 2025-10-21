/**
 * InstaMATPluginStyle.cpp (InstaMAT)
 *
 * Copyright 2016-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATPluginStyle.cpp
 * @copyright 2016-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATPCH.h"
#include "Slate/InstaMATPluginStyle.h"

#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Styling/SegmentedControlStyle.h"

TSharedPtr<FSlateStyleSet> FInstaMATPluginStyle::StyleInstance = nullptr;

void FInstaMATPluginStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FInstaMATPluginStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FInstaMATPluginStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("InstaMATPluginStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon14x14(14.0, 14.0);
const FVector2D Icon16x16(16.0, 16.0);
const FVector2D Icon18x18(18.0, 18.0);
const FVector2D Icon20x20(20.0, 20.0);
const FVector2D Icon24x24(24.0, 24.0);
const FVector2D Icon40x40(40.0, 40.0);
const FVector2D Icon48x48(48.0, 48.0);
const FVector2D Icon32x32(32.0, 32.0);
const FVector2D Icon80x80(80.0, 80.0);
const FVector2D Icon37x20(37.0, 20.0);
const FVector2D Icon256x256(256.0, 256.0);

TSharedRef<FSlateStyleSet> FInstaMATPluginStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(TEXT("InstaMATPluginStyle")));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("InstaMAT"))->GetBaseDir() / TEXT("Resources"));
	
	const FSlateFontInfo BoldFont = TTF_FONT(TEXT("Fonts/Roboto-Bold"), 10);
	const FSlateFontInfo NormalFont = TTF_FONT(TEXT("Fonts/Roboto-Regular"), 10);
	const FSlateFontInfo SmallFont = TTF_FONT(TEXT("Fonts/Roboto-Regular"), 7);
	
	const FLinearColor DarkGray(0.37f, 0.37f, 0.37f);
	const FLinearColor LightGray(0.85f, 0.85f, 0.85f);
	const FLinearColor VeryDarkGray(0.025f, 0.025f, 0.025f);
	const FLinearColor ShadeGray(0.0137f, 0.0137f, 0.0137f);
	const FLinearColor EditorInstaMATBlue(FColor(4, 169, 199));
	const FLinearColor EditorInstaMATBlueHighlight(FColor(77, 216, 240));
	const FLinearColor EditorInstaMATBlueDarkened(FColor(35, 98, 110));
	const FLinearColor SideBarLightGrayColor(FColor(69, 69, 69));
	const FLinearColor SideBarDarkGrayColor(FColor(40, 40, 40));
	const FLinearColor SideBarIconNormalColor(FColor(204, 204, 204));
	const FLinearColor RefreshingLicenseWindowMainColor(FColor(24, 24, 24));
	const FLinearColor RefreshingLicenseWindowMainOutlineColor(FColor(56, 59, 60));

	const FColor InstaMATBackgroundGray = FColor(8, 8, 8);
	const FColor InstaMATBackgroundDarkGray = FColor(5, 5, 5);
	const FColor InstaMATBackgroundLightGray = FColor(16, 16, 16);
	const FColor ContextMenuBackground = FColor(2, 2, 2);
	const FColor InstaMATSeparatorColor = FColor(69, 69, 69);

	const float kBigBorderRadius = 12.5f;
	const float kSplitterBorderRadius = 3.0f;
	const float kDefaultBorderRadius = 5.0f;
	const float kButtonBorderThickness = 0.8f;
	const float kGraphBrowserBorderThickness = 1.5f;
	const float kGraphBrowserBorderRadius = 8.0f;

	Style->Set(TEXT("InstaMAT.PluginAction"), new IMAGE_BRUSH(TEXT("InstaMATIcon_40x40"), Icon40x40));
	Style->Set(TEXT("InstaMATUI.OpenInstaMATWindow"), new IMAGE_BRUSH(TEXT("InstaMATIcon_80x80"), Icon40x40));
	Style->Set(TEXT("InstaMATUI.OpenInstaMATWindow.Small"), new IMAGE_BRUSH(TEXT("InstaMATIcon_40x40"), Icon20x20));
	Style->Set(TEXT("InstaMATUI.Browser.Materials.Normal"), new IMAGE_BRUSH(TEXT("InstaMAT_Materials"), Icon16x16, SideBarIconNormalColor));
	Style->Set(TEXT("InstaMATUI.Browser.Materials.Hovered"), new IMAGE_BRUSH(TEXT("InstaMAT_Materials"), Icon16x16));
	Style->Set(TEXT("InstaMATUI.Browser.Mesh.Normal"), new IMAGE_BRUSH(TEXT("InstaMAT_Mesh"), Icon16x16, SideBarIconNormalColor));
	Style->Set(TEXT("InstaMATUI.Browser.Mesh.Hovered"), new IMAGE_BRUSH(TEXT("InstaMAT_Mesh"), Icon16x16));
	Style->Set(TEXT("InstaMATUI.Browser.NoFilter.Normal"), new IMAGE_BRUSH(TEXT("InstaMAT_NoFilter"), Icon16x16, SideBarIconNormalColor));
	Style->Set(TEXT("InstaMATUI.Browser.NoFilter.Hovered"), new IMAGE_BRUSH(TEXT("InstaMAT_NoFilter"), Icon16x16));
	Style->Set(TEXT("InstaMATUI.Browser.User.Normal"), new IMAGE_BRUSH(TEXT("InstaMAT_User"), Icon16x16, SideBarIconNormalColor));
	Style->Set(TEXT("InstaMATUI.Browser.User.Hovered"), new IMAGE_BRUSH(TEXT("InstaMAT_User"), Icon16x16));
	Style->Set(TEXT("InstaMATUI.TabIcon"), new IMAGE_BRUSH(TEXT("InstaMATIcon_40x40"), Icon18x18));
	Style->Set(TEXT("InstaMATUI.MediumIcon"), new IMAGE_BRUSH(TEXT("InstaMATIcon_80x80"), Icon80x80));
	Style->Set(TEXT("InstaMATUI.BigIcon"), new IMAGE_BRUSH(TEXT("InstaMATIcon_256x256"), Icon256x256));
	Style->Set(TEXT("InstaMATUI.Input"), new IMAGE_BRUSH(TEXT("InstaMAT_Input_32x32"), Icon14x14));
	Style->Set(TEXT("InstaMATUI.Collapsible.Collapsed"), new IMAGE_BRUSH(TEXT("InstaMAT_AngleRight"), Icon14x14));
	Style->Set(TEXT("InstaMATUI.Collapsible.Uncollapsed"), new IMAGE_BRUSH(TEXT("InstaMAT_AngleDown"), Icon14x14));
	Style->Set(TEXT("InstaMATUI.Browser.Directory.Normal"), new IMAGE_BRUSH(TEXT("InstaMAT_Directory"), Icon16x16, SideBarIconNormalColor));
	Style->Set(TEXT("InstaMATUI.Browser.Directory.Hovered"), new IMAGE_BRUSH(TEXT("InstaMAT_Directory"), Icon16x16));
	Style->Set(TEXT("InstaMATUI.Browser.Table.Normal"), new IMAGE_BRUSH(TEXT("InstaMAT_Table"), Icon16x16, SideBarIconNormalColor));
	Style->Set(TEXT("InstaMATUI.Browser.Table.Hovered"), new IMAGE_BRUSH(TEXT("InstaMAT_Table"), Icon16x16));
	Style->Set(TEXT("InstaMATUI.Browser.RowIcon.Normal"), new IMAGE_BRUSH(TEXT("InstaMAT_RowIcon"), Icon16x16, SideBarIconNormalColor));
	Style->Set(TEXT("InstaMATUI.Browser.RowIcon.Hovered"), new IMAGE_BRUSH(TEXT("InstaMAT_RowIcon"), Icon16x16));
	Style->Set(TEXT("InstaMATUI.Browser.Tiles.Normal"), new IMAGE_BRUSH(TEXT("InstaMAT_Tiles"), Icon16x16, SideBarIconNormalColor));
	Style->Set(TEXT("InstaMATUI.Browser.Tiles.Hovered"), new IMAGE_BRUSH(TEXT("InstaMAT_Tiles"), Icon16x16));
	Style->Set(TEXT("InstaMATUI.Button.Border.White"), new FSlateRoundedBoxBrush(FLinearColor::Transparent, kBigBorderRadius, FLinearColor::White, kButtonBorderThickness));
	Style->Set(TEXT("InstaMATUI.Button.Border.White.Hover"), new FSlateRoundedBoxBrush(ShadeGray, kBigBorderRadius, VeryDarkGray, kButtonBorderThickness));
	Style->Set(TEXT("InstaMATUI.Button.Border.White.Pressed"), new FSlateRoundedBoxBrush(EditorInstaMATBlue, kBigBorderRadius, EditorInstaMATBlue, kButtonBorderThickness));
	Style->Set(TEXT("InstaMATUI.Collapsible.HeaderBrush"), new FSlateColorBrush(InstaMATBackgroundDarkGray));
	Style->Set(TEXT("InstaMATUI.Browser.Background"), new FSlateColorBrush(InstaMATBackgroundGray));
	Style->Set(TEXT("InstaMATUI.BackgroundGray"), InstaMATBackgroundDarkGray);
	Style->Set(TEXT("InstaMATUI.BackgroundGray.Brush"), new FSlateColorBrush(InstaMATBackgroundDarkGray));
	Style->Set(TEXT("InstaMATUI.ContextMenu.Background"), new FSlateColorBrush(ContextMenuBackground));
	Style->Set(TEXT("InstaMATUI.SideBar.Background.Brush.Light"), new FSlateRoundedBoxBrush(SideBarLightGrayColor, 1.0f, FLinearColor::Transparent, 0.0f));
	Style->Set(TEXT("InstaMATUI.SideBar.BlueBorder"), new FSlateRoundedBoxBrush(EditorInstaMATBlue, 1.0f, FLinearColor::Transparent, 0.0f));
	Style->Set(TEXT("InstaMATUI.SideBar.Background.Brush.Dark"), new FSlateColorBrush(SideBarDarkGrayColor));
	Style->Set(TEXT("InstaMATUI.SideBar.Separator.Brush"), new FSlateRoundedBoxBrush(FSlateColor(InstaMATSeparatorColor), 1.0f, FLinearColor::White, 0.0f));

	Style->Set(TEXT("InstaMATUI.Icons.Info"), new IMAGE_BRUSH(TEXT("InstaMAT_Help"), Icon20x20));
	Style->Set(TEXT("InstaMATUI.Icons.World"), new IMAGE_BRUSH(TEXT("InstaMAT_World"), Icon24x24));
	Style->Set(TEXT("InstaMATUI.Icons.Youtube"), new IMAGE_BRUSH(TEXT("InstaMAT_Youtube"), Icon24x24));
	Style->Set(TEXT("InstaMATUI.Icons.Question"), new IMAGE_BRUSH(TEXT("InstaMAT_Question"), Icon24x24));
	Style->Set(TEXT("InstaMATUI.Icons.Book"), new IMAGE_BRUSH(TEXT("InstaMAT_Book"), Icon24x24));
	Style->Set(TEXT("InstaMATUI.Icons.Speech"), new IMAGE_BRUSH(TEXT("InstaMAT_Speech"), Icon24x24));
	Style->Set(TEXT("InstaMATUI.Icons.X"), new IMAGE_BRUSH(TEXT("InstaMAT_X"), Icon24x24));
	Style->Set(TEXT("InstaMATUI.Icons.Download"), new IMAGE_BRUSH(TEXT("InstaMAT_Download"), Icon24x24));
	Style->Set(TEXT("InstaMATUI.Icons.Discord"), new IMAGE_BRUSH(TEXT("InstaMAT_Discord"), Icon24x24));
	Style->Set(TEXT("InstaMATUI.Checked"), new IMAGE_BRUSH(TEXT("InstaMAT_Checked"), Icon37x20));
	Style->Set(TEXT("InstaMATUI.Unchecked"), new IMAGE_BRUSH(TEXT("InstaMAT_Unchecked"), Icon37x20));
	Style->Set(TEXT("InstaMATUI.Icons.Warning"), new IMAGE_BRUSH(TEXT("InstaMAT_Warning"), Icon20x20));

	const FSlateRoundedBoxBrush TileBackgroundBrushRounded = FSlateRoundedBoxBrush(FSlateColor(InstaMATBackgroundLightGray), kGraphBrowserBorderRadius, VeryDarkGray, kButtonBorderThickness);
	const FSlateRoundedBoxBrush TileBackgroundHoveredBrushRounded = FSlateRoundedBoxBrush(FLinearColor::Transparent, kGraphBrowserBorderRadius, FSlateColor(FLinearColor(0.0016f, 0.66f, 0.78f)), kGraphBrowserBorderThickness);
	Style->Set(TEXT("InstaMATUI.Browser.Tiles"), FTableRowStyle()
		.SetActiveBrush(TileBackgroundHoveredBrushRounded)
		.SetInactiveBrush(TileBackgroundBrushRounded)
		.SetInactiveHoveredBrush(TileBackgroundHoveredBrushRounded)
		.SetOddRowBackgroundBrush(FSlateColorBrush(FLinearColor::Transparent))
		.SetEvenRowBackgroundBrush(FSlateColorBrush(FLinearColor::Transparent))
		.SetInactiveHighlightedBrush(TileBackgroundBrushRounded)
		.SetActiveHoveredBrush(TileBackgroundHoveredBrushRounded)
		.SetActiveHighlightedBrush(TileBackgroundBrushRounded)
		.SetOddRowBackgroundHoveredBrush(TileBackgroundHoveredBrushRounded)
		.SetEvenRowBackgroundHoveredBrush(TileBackgroundHoveredBrushRounded)
		.SetSelectorFocusedBrush(TileBackgroundBrushRounded)
		);

	Style->Set(TEXT("InstaMATUI.Browser.Category"), FTableViewStyle()
		.SetBackgroundBrush(FSlateColorBrush(FColor(8, 8, 8)))
	);

	const FSlateRoundedBoxBrush CategoryBackgroundBrushRounded = FSlateRoundedBoxBrush(FSlateColor(InstaMATBackgroundLightGray), kGraphBrowserBorderRadius, FSlateColor(InstaMATBackgroundLightGray), kButtonBorderThickness);
	const FSlateRoundedBoxBrush CategoryBackgroundHoveredBrushRounded = FSlateRoundedBoxBrush(FLinearColor::Transparent, kGraphBrowserBorderRadius, FLinearColor::Transparent, kGraphBrowserBorderThickness);
	Style->Set(TEXT("InstaMATUI.Browser.Category.Row"), FTableRowStyle(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"))
		.SetActiveBrush(CategoryBackgroundHoveredBrushRounded)
		.SetInactiveBrush(CategoryBackgroundBrushRounded)
		.SetInactiveHoveredBrush(CategoryBackgroundHoveredBrushRounded)
		.SetOddRowBackgroundBrush(FSlateColorBrush(FLinearColor::Transparent))
		.SetEvenRowBackgroundBrush(FSlateColorBrush(FLinearColor::Transparent))
		.SetInactiveHighlightedBrush(CategoryBackgroundBrushRounded)
		.SetActiveHoveredBrush(CategoryBackgroundHoveredBrushRounded)
		.SetActiveHighlightedBrush(CategoryBackgroundBrushRounded)
		.SetOddRowBackgroundHoveredBrush(CategoryBackgroundHoveredBrushRounded)
		.SetEvenRowBackgroundHoveredBrush(CategoryBackgroundHoveredBrushRounded)
		.SetSelectorFocusedBrush(CategoryBackgroundBrushRounded)
		.SetSelectedTextColor(FSlateColor(FLinearColor::White))
	);

	Style->Set(TEXT("InstaMATUI.Browser.Segment"), FSegmentedControlStyle(FAppStyle::Get().GetWidgetStyle<FSegmentedControlStyle>("SegmentedControl"))
		.SetBackgroundBrush(FSlateColorBrush(VeryDarkGray))
	);

	const FEditableTextBoxStyle& DefaultStyle = FAppStyle::GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");
	Style->Set(TEXT("InstaMATUI.EditableText"), FEditableTextBoxStyle(DefaultStyle)
		.SetBackgroundImageReadOnly(DefaultStyle.BackgroundImageNormal)
	);

	Style->Set(TEXT("InstaMATUI.Scrollbar"), FScrollBarStyle(FAppStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar"))
		.SetThickness(3.0f));

	Style->Set(TEXT("InstaMATUI.Splitter"), FSplitterStyle()
		.SetHandleNormalBrush(FSlateRoundedBoxBrush(VeryDarkGray, kSplitterBorderRadius, FLinearColor::Transparent, 0.0f))
		.SetHandleHighlightBrush(FSlateRoundedBoxBrush(LightGray, kSplitterBorderRadius, FLinearColor::Transparent, 0.0f))
	);

	const FSlateRoundedBoxBrush HelpButtonBackground = FSlateRoundedBoxBrush(FSlateColor(ShadeGray), kDefaultBorderRadius, FSlateColor(ShadeGray), kDefaultBorderRadius);
	Style->Set(TEXT("InstaMATUI.HelpButton"), FButtonStyle()
		.SetPressedForeground(FSlateColor(FLinearColor::White))
		.SetNormal(HelpButtonBackground)
		.SetHovered(HelpButtonBackground)
		.SetPressed(HelpButtonBackground) 
	);

	Style->Set(TEXT("InstaMATUI.BoldFont"), BoldFont);

	FExpandableAreaStyle ExpandableStyle;
	ExpandableStyle.SetCollapsedImage(*(new IMAGE_BRUSH(TEXT("InstaMAT_AngleRight"), Icon14x14)));
	ExpandableStyle.SetExpandedImage(*(new IMAGE_BRUSH(TEXT("InstaMAT_AngleDown"), Icon14x14)));
	Style->Set(TEXT("InstaMATUI.Collapsible"), ExpandableStyle);

	Style->Set(TEXT("InstaMATUI.Context.Item"), FButtonStyle()
		.SetNormalForeground(FSlateColor(FLinearColor::White))
		.SetDisabledForeground(FSlateColor(FLinearColor::White))
		.SetHoveredForeground(FSlateColor(FLinearColor::White))
		.SetPressedForeground(FSlateColor(FLinearColor::White))
		.SetHovered(FSlateColorBrush(InstaMATBackgroundDarkGray))
		.SetPressed(FSlateColorBrush(InstaMATBackgroundDarkGray))
		.SetNormal(FSlateColorBrush(ContextMenuBackground))
	);

	{
		Style->Set(TEXT("InstaMATUI.ButtonPrimary"), FButtonStyle()
			.SetNormal(BOX_BRUSH(TEXT("FlatButton"), 2.0f / 8.0f, EditorInstaMATBlue))
			.SetHovered(BOX_BRUSH(TEXT("FlatButton"), 2.0f / 8.0f, EditorInstaMATBlueHighlight))
			.SetPressed(BOX_BRUSH(TEXT("FlatButton"), 2.0f / 8.0f, EditorInstaMATBlueDarkened))
			.SetNormalPadding(FMargin(2.0f, 2.0f, 2.0f, 2.0f))
			.SetPressedPadding(FMargin(2.0f, 3.0f, 2.0f, 1.0f)));
		
		Style->Set(TEXT("InstaMATUI.ButtonPrimary.BoldTextStyle"), FTextBlockStyle()
			.SetFont(BoldFont)
			.SetColorAndOpacity(LightGray)
			.SetShadowOffset(FVector2D(1.0, 1.0))
			.SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f))
			.SetShadowOffset(FVector2D::ZeroVector)
			.SetShadowColorAndOpacity(FLinearColor::Black)
			.SetHighlightColor(FLinearColor::White));
		
		Style->Set(TEXT("InstaMATUI.ButtonPrimary.DefaultTextStyle"), FTextBlockStyle()
			.SetFont(NormalFont)
			.SetColorAndOpacity(VeryDarkGray)
			.SetShadowOffset(FVector2D(1.0, 1.0))
			.SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f))
			.SetShadowOffset(FVector2D::ZeroVector)
			.SetShadowColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
			.SetHighlightColor(FLinearColor::White));
	}
	
	const FTextBlockStyle NormalText = FTextBlockStyle()
		.SetFont(NormalFont)
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetShadowOffset(FVector2D::ZeroVector)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		.SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f));
	
	{
		const FTextBlockStyle HelpNormal = FTextBlockStyle(NormalText)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Regular"), 12.0f))
			.SetShadowOffset(FVector2D::UnitVector);

		Style->Set(TEXT("InstaMAT.Normal"), HelpNormal);

		Style->Set(TEXT("InstaMATUI.Library.Small"), FTextBlockStyle(NormalText)
			.SetFont(SmallFont)
			.SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f))
			));

		Style->Set(TEXT("InstaMAT.Bold"), FTextBlockStyle(HelpNormal)
			.SetShadowOffset(FVector2D::ZeroVector)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 10.0f)));

		Style->Set(TEXT("InstaMAT.Strong"), FTextBlockStyle(HelpNormal)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 12.0f))
			.SetShadowOffset(FVector2D::UnitVector));

		Style->Set(TEXT("InstaMAT.H1"), FTextBlockStyle(HelpNormal)
			.SetColorAndOpacity(EditorInstaMATBlue)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 36.0f))
			.SetShadowOffset(FVector2D::UnitVector));

		Style->Set(TEXT("InstaMAT.H2"), FTextBlockStyle(HelpNormal)
			.SetColorAndOpacity(EditorInstaMATBlue)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 30.0f))
			.SetShadowOffset(FVector2D::UnitVector));

		Style->Set(TEXT("InstaMATMeshReduction.H3"), FTextBlockStyle(HelpNormal)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 24.0f))
			.SetColorAndOpacity(EditorInstaMATBlue)
			.SetShadowOffset(FVector2D::UnitVector));

		Style->Set(TEXT("InstaMAT.H4"), FTextBlockStyle(HelpNormal)
			.SetColorAndOpacity(EditorInstaMATBlue)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 18.0f))
			.SetShadowOffset(FVector2D::UnitVector));

		Style->Set(TEXT("InstaMAT.H5"), FTextBlockStyle(HelpNormal)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 12.0f))
			.SetShadowOffset(FVector2D::UnitVector));

		Style->Set(TEXT("InstaMAT.H6"), FTextBlockStyle(HelpNormal)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 10.0f))
			.SetShadowOffset(FVector2D::UnitVector));

		const FTextBlockStyle LinkText = FTextBlockStyle(NormalText)
			.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 12.0f))
			.SetColorAndOpacity(EditorInstaMATBlue)
			.SetShadowOffset(FVector2D::UnitVector);

		const FButtonStyle HoverOnlyHyperlinkButton = FButtonStyle()
			.SetNormal(FSlateNoResource())
			.SetPressed(FSlateNoResource())
			.SetHovered(BORDER_BRUSH(TEXT("Old/HyperlinkUnderline"), FMargin(0.0f, 0.0f, 0.0f, 3.0f / 16.0f)));

		const FHyperlinkStyle HoverOnlyHyperlink = FHyperlinkStyle()
			.SetUnderlineStyle(HoverOnlyHyperlinkButton)
			.SetTextStyle(LinkText)
			.SetPadding(FMargin(0.0f));

		Style->Set("InstaMAT.FloatingLicense.Window.Title", FTextBlockStyle()
			.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 18))
			.SetColorAndOpacity(FSlateColor(FColor::White)));

		Style->Set(TEXT("InstaMAT.Hyperlink"), HoverOnlyHyperlink);

		Style->Set(TEXT("InstaMAT.HeaderImage"), FInlineTextImageStyle()
			.SetImage(IMAGE_BRUSH(TEXT("InstaMAT_Logo_880x174"), FVector2D(880.0, 187.0)))
			.SetBaseline(0));

		Style->Set(TEXT("InstaMAT.FooterImage"), FInlineTextImageStyle()
			.SetImage(IMAGE_BRUSH(TEXT("InstaMAT_Logo_302x60"), FVector2D(302.0, 64.0)))
			.SetBaseline(0));

		static FWindowStyle FloatingLicenseWindowStyle = FWindowStyle::GetDefault();
		FloatingLicenseWindowStyle.SetBackgroundBrush(FSlateRoundedBoxBrush(RefreshingLicenseWindowMainColor, 0.0f, RefreshingLicenseWindowMainColor, 0.0f));
		FloatingLicenseWindowStyle.SetCornerRadius(50.0f);
		FloatingLicenseWindowStyle.SetOutlineBrush(FSlateNoResource());
		FloatingLicenseWindowStyle.SetBorderBrush(FSlateRoundedBoxBrush(RefreshingLicenseWindowMainColor, 27.0f, RefreshingLicenseWindowMainOutlineColor, 2.0f));

		Style->Set("InstaMAT.FloatingLicense.Window.Style", FloatingLicenseWindowStyle);
		Style->Set("InstaMAT.FloatingLicense.Button.Normal", new FSlateRoundedBoxBrush(RefreshingLicenseWindowMainColor, 18.0f, FSlateColor(FColor::White), 1.0f));

		Style->Set(TEXT("InstaMAT.LogoLarge"), new IMAGE_BRUSH(TEXT("InstaMAT_Logo_880x174"), FVector2D(880.0, 174.0)));
		Style->Set(TEXT("InstaMAT.LogoMedium"), new IMAGE_BRUSH(TEXT("InstaMAT_Logo_512x101"), FVector2D(512.0, 101.0)));
		Style->Set(TEXT("InstaMAT.LogoSmall"), new IMAGE_BRUSH(TEXT("InstaMAT_Logo_302x60"), FVector2D(302.0, 60.0)));
		Style->Set(TEXT("InstaMAT.LogoTiny"), new IMAGE_BRUSH(TEXT("InstaMAT_Logo_126x25"), FVector2D(126.0, 25.0)));

		// Meta Data style
		{
			const float SmallTextSize = 10.0f;

			const FTextBlockStyle MetaDataNameText = FTextBlockStyle(NormalText)
				.SetColorAndOpacity(FSlateColor(LightGray))
				.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 22.0f));

			const FTextBlockStyle MetaDataCategoryText = FTextBlockStyle(NormalText)
				.SetColorAndOpacity(FSlateColor(DarkGray))
				.SetFont(TTF_FONT(TEXT("Fonts/Roboto-Bold"), 18.0f));

			const FTextBlockStyle MetaDataDescriptionText = FTextBlockStyle(NormalText)
				.SetColorAndOpacity(FSlateColor(LightGray))
				.SetFontSize(SmallTextSize);

			const FTextBlockStyle MetaDataAboutText = FTextBlockStyle(MetaDataDescriptionText)
				.SetColorAndOpacity(FSlateColor(DarkGray))
				.SetFontSize(SmallTextSize);

			Style->Set(TEXT("InstaMAT.MetaData.Text.Name"), MetaDataNameText);
			Style->Set(TEXT("InstaMAT.MetaData.Text.Category"), MetaDataCategoryText);
			Style->Set(TEXT("InstaMAT.MetaData.Text.Documentation"), MetaDataDescriptionText);
			Style->Set(TEXT("InstaMAT.MetaData.Text.About"), MetaDataAboutText);
		}
	}

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FInstaMATPluginStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FInstaMATPluginStyle::Get()
{
	return *StyleInstance;
}