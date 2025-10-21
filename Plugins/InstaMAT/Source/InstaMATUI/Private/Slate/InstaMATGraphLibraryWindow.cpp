/**
 * SInstaMATGraphLibraryWindow.cpp (InstaMAT)
 *
 * Copyright 2019-2021 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file SInstaMATGraphLibraryWindow.cpp
 * @copyright 2019-2021 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATGraphLibraryWindow.h"
#include "InstaMATUIPCH.h"

#include "Slate/InstaMATPluginStyle.h"
#include "Slate/InstaMATContextMenu.h"
#include "Slate/InstaMATSideBarButton.h"
#include "Slate/InstaMATSegementedControl.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "Widgets/Layout/SScaleBox.h"

#include "InstaMATModule.h"
#include "InstaMATImporter/Public/InstaMATImporterFactory.h"
#include "LevelEditor.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "IDocumentation.h"
#include "Algo/ForEach.h"

#define LOCTEXT_NAMESPACE "InstaMATUI"

/**< Categories that are filtered out. */
const static TArray<FString> kInstaMATCategoryFilter = {
	TEXT("default"),
	TEXT("internal"),
	TEXT("test"),
	TEXT("assets/"),
	TEXT("demo"),
	TEXT("elementexpression"),
	TEXT("paintbrush"),
	TEXT("floodfill"),
	TEXT("pointcloud"),
	TEXT("triangulator"),
	TEXT("neural"),
	TEXT("tutorials")
};

/**
 * The InstaMATGraphLibraryWindowUtilities namespace contains utility functions
 * for the Graph Library Window.
 */
namespace InstaMATGraphLibraryWindowUtilities
{
	/**
	 * Sets the provided \p Item and its children visible recursively.
	 * 
	 * @param Item The item to set visible.
	 * @param bIsVisible The visibility status.
	 * @param Depth The recursion depth.
	 */
	static void SetCategoryItemsVisibilityStatusRecursive(const TSharedPtr<FCategoryHierarchyNode>& Item, const bool bIsVisible, const uint32 Depth)
	{
		static const uint32 kMaximumHierarchyDepth = 512u;

		Item->bIsVisible = bIsVisible;

		if (Depth >= kMaximumHierarchyDepth)
		{
			UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Reached maximum child depth."));
			return;
		}

		for (const TSharedPtr<FCategoryHierarchyNode>& ChildItem : Item->Children)
		{
			InstaMATGraphLibraryWindowUtilities::SetCategoryItemsVisibilityStatusRecursive(ChildItem, bIsVisible, Depth + 1u);
		}
	}

	/**
	 * Sets the category visibility based on the filter array.
	 * 
	 * @param Roots The root node array.
	 */
	static void SetVisibilityBasedOnFilterArray(TArray<TSharedPtr<FCategoryHierarchyNode>>& Roots)
	{
		for (const TSharedPtr<FCategoryHierarchyNode>& Root : Roots)
		{
			for (const FString& FilterValue : kInstaMATCategoryFilter)
			{
				if (Root->Category.StartsWith(FilterValue, ESearchCase::IgnoreCase))
				{
					InstaMATGraphLibraryWindowUtilities::SetCategoryItemsVisibilityStatusRecursive(Root, /*bIsVisible:*/ false, /*Depth:*/ 0u);
					break;
				}
			}
		}
	}

	/**
	 * Builds the category hierarchy based on the provided categories array.
	 * 
	 * @param Categories The category array.
	 * @param [out] OutCategoryMap The hierarchy container.
	 */
	static void BuildCategoryHierarchy(const TArray<FString>& Categories, TMap<FString, TSharedPtr<FCategoryHierarchyNode>>& OutCategoryMap)
	{
		// Build category hierarchy
		for (const FString& Category : Categories)
		{
			// Category string contains a hierarchical value
			if (Category.Contains(TEXT("/")))
			{
				// Split into single name categories
				TArray<FString> Tokens;
				Category.ParseIntoArray(Tokens, TEXT("/"));

				// Go through the chain and add a node for each item into the map
				FString FullPath;
				for (const FString& Token : Tokens)
				{
					check(!Token.IsEmpty());

					const FString ParentPath = FullPath;

					FullPath = FullPath.IsEmpty() ? Token : FString::Printf(TEXT("%s/%s"), *ParentPath, *Token);

					if (OutCategoryMap.Contains(FullPath))
						continue;

					const TSharedPtr<FCategoryHierarchyNode> Node = TSharedPtr<FCategoryHierarchyNode>(new FCategoryHierarchyNode);
					Node->FullCategory = FullPath;
					Node->Category = Token;
					Node->bIsVisible = true;

					if (!ParentPath.IsEmpty())
					{
						// Create node relation to parent objects
						if (OutCategoryMap.Contains(ParentPath))
						{
							OutCategoryMap[ParentPath]->Children.AddUnique(Node);
						}
					}

					OutCategoryMap.Add(FullPath, Node);
				}
			}
			else
			{
				// create
				if (OutCategoryMap.Contains(Category))
					continue;

				const TSharedPtr<FCategoryHierarchyNode> Node = TSharedPtr<FCategoryHierarchyNode>(new FCategoryHierarchyNode);
				Node->Category = Category;
				Node->FullCategory = Category;
				OutCategoryMap.Add(Category, Node);
			}
		}
	}
};

SInstaMATGraphLibraryWindow::SInstaMATGraphLibraryWindow() : 
SCompoundWidget(),
MaterialFilteredCategories(),
CategoryRoots(),
CategoryRootsSource(),
OriginalCategoryRoots(),
GraphDataSource(),
OriginalDataSource(),
SearchField(nullptr),
CategoryView(nullptr),
GraphView(nullptr),
LastSelectedCategory(nullptr),
PreviewWidget(nullptr),
Filter(EInstaMATLibraryFilter::InstaMAT_Material),
RowType(EInstaMATLibraryGraphListRowType::InstaMAT_TileMediumIcon),
bIsSearchInSelectedCategoryActive(false)
{
}

SInstaMATGraphLibraryWindow::~SInstaMATGraphLibraryWindow()
{
	CategoryRoots.Empty();
	OriginalCategoryRoots.Empty();
}

void SInstaMATGraphLibraryWindow::OnCategorySelectionChanged(TSharedPtr<FCategoryHierarchyNode> SelectedNode, ESelectInfo::Type SelectionType)
{
	LastSelectedCategory = SelectedNode;
	GraphDataSource.Reset();

	if (SelectedNode != nullptr)
	{
		GraphDataSource = SelectedNode->Graphs;
		
		CategoryView->RequestScrollIntoView(SelectedNode, 0u);
	}

	// Remove all non user graphs
	if (Filter == EInstaMATLibraryFilter::InstaMAT_User)
	{
		GraphDataSource.RemoveAll([](const TSharedPtr<FInstaMATGraphObjectViewItem>& Item) { return !Item->bIsUserGraph; });
	}

	SInstaMATGraphLibraryWindow::SortDataSource(GraphDataSource);
	GraphView->RebuildList();

	if (!SearchField->GetText().IsEmpty())
	{
		OnSearchTextChanged(SearchField->GetText());
	}
}

void SInstaMATGraphLibraryWindow::OnGraphSelectionChanged(TSharedPtr<FInstaMATGraphObjectViewItem> SelectedNode, ESelectInfo::Type SelectionType)
{
	if (PreviewWidget == nullptr)
		return;

	if (SelectedNode != nullptr)
	{
		if (!SelectedNode->GraphPreviewBrush.IsValid() && SelectedNode->Preview != nullptr)
		{
			SelectedNode->GraphPreviewBrush = MakeShared<FSlateDynamicImageBrush>(SelectedNode->Preview, FVector2D(256.0, 256.0), FName(TEXT("InstaMAT")));
		}
		
		GraphView->RequestScrollIntoView(SelectedNode, 0u);
	}

	PreviewWidget->SetPreviewItem(SelectedNode);
}

void SInstaMATGraphLibraryWindow::OnSearchTextChanged(const FText& Text)
{
	const FString SearchText = Text.ToString();
	TSharedPtr<FCategoryHierarchyNode> LastValidSelectedCategory = LastSelectedCategory;
	GraphView->ClearSelection();

	bool bIsSearchActive = !SearchText.IsEmpty();

	SetCategoriesUIVisibility(!bIsSearchActive || (bIsSearchActive && bIsSearchInSelectedCategoryActive));

	if (!bIsSearchActive)
	{
		OnFilterSelectionChanged(Filter);
		return;
	}

	// Update GraphView
	GraphDataSource.Reset();

	// Go through all graphs currently available based on the Filter and search for string in name.
	for (const TSharedPtr<FCategoryHierarchyNode>& Item : CategoryRootsSource)
	{
		if(!Item->bIsVisible)
			continue;

		for (const TSharedPtr<FInstaMATGraphObjectViewItem>& Graph : Item->Graphs)
		{
			check(Graph != nullptr);

			if (Filter == EInstaMATLibraryFilter::InstaMAT_User && !Graph->bIsUserGraph)
				continue;

			if (bIsSearchInSelectedCategoryActive)
			{
				if (!LastValidSelectedCategory.IsValid())
					continue;
				
				if (!Graph->Category.StartsWith(LastValidSelectedCategory->FullCategory))
					continue;

				const FString SubString = Graph->Category.Mid(LastValidSelectedCategory->FullCategory.Len());
				if (!SubString.IsEmpty() && !SubString.StartsWith(TEXT("/")))
					continue;
			}

			if (Graph->GraphFriendlyName.Contains(SearchText, ESearchCase::IgnoreCase))
			{
				GraphDataSource.AddUnique(Graph);
			}
		}
	}

	SInstaMATGraphLibraryWindow::SortDataSource(GraphDataSource);
	GraphView->RebuildList();
}

void SInstaMATGraphLibraryWindow::Construct(const FArguments& InArgs)
{
	if (!InArgs._GraphObjects.IsSet())
		return;

	constexpr float kListRowPadding = 3.0f;

	CategoryRoots.Reset();
	OriginalCategoryRoots.Reset();
	UserCategories.Reset();
	OriginalDataSource = InArgs._GraphObjects.Get();

	// Sort graph objects
	SInstaMATGraphLibraryWindow::SortDataSource(GraphDataSource);

	/// the fnOnGraphFieldSearchChanged lambda filters the GraphView datasource for the specified search text.
	const auto fnOnGraphFieldSearchChanged = [this](const FText& InText)
	{
		const FString SearchText = InText.ToString();

		GraphDataSource.Reset();

		if (SearchText.IsEmpty())
		{
			GraphDataSource.Append(OriginalDataSource);
		}
		else
		{
			for (const auto& Item : OriginalDataSource)
			{
				if (Item->GraphFriendlyName.Contains(SearchText, ESearchCase::IgnoreCase))
				{
					GraphDataSource.Add(Item);
				}
			}
		}
		
		SInstaMATGraphLibraryWindow::SortDataSource(GraphDataSource);
		GraphView->RebuildList();
	};
	
	/// The fnGenerateCategoryViewRow lambda creates a row for the CategoryView.
	const auto fnGenerateCategoryViewRow = [kListRowPadding](TSharedPtr<FCategoryHierarchyNode> Item, const TSharedRef<STableViewBase>& Table)
	{
		check(Item != nullptr);

		/// The fnIsItemVisible lambda determines whether the item is visible.
		const auto fnIsItemVisible = [/*copy:*/ Item]() -> EVisibility
		{
			return Item->bIsVisible ? EVisibility::Visible : EVisibility::Collapsed;
		};

		return	SNew(STableRow<TSharedPtr<FCategoryHierarchyNode>>, Table)
				.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("InstaMATUI.Browser.Category.Row")))
				.Visibility_Lambda(fnIsItemVisible)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().Padding(kListRowPadding)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Item->Category))
					]
				];
	};

	/// The fnOnItemDoubleClicked lambda handles a item double click selection.
	const auto fnOnItemDoubleClicked = [this](TSharedPtr<FInstaMATGraphObjectViewItem> Item)
	{
		UInstaMATImporterFactory::ImportGraphObjectWithID(Item->GraphID);
	};

	// Retrieve categories from InstaMAT API and build hierarchy based on those.
	FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	IInstaMAT* const InstaMATInterface = InstaMATModule.GetInstaMATInterface();
	check(InstaMATInterface);

	const TArray<FString>& Categories = InstaMATInterface->GetCategories(/*bEnforceUpdate:*/ true);
	TMap<FString, TSharedPtr<FCategoryHierarchyNode>> CategoryMap;

	InstaMATGraphLibraryWindowUtilities::BuildCategoryHierarchy(Categories, CategoryMap);
	
	// Find Root objects in the category hierarchy
	for (const auto& [_, Value] : CategoryMap)
	{
		if (!Value->FullCategory.Contains(TEXT("/")))
		{
			OriginalCategoryRoots.AddUnique(Value);
		}
	}

	// Assign matching graph items to hierarchy
	for (const auto& [_, Value] : CategoryMap)
	{
		const FString& SearchCategory = Value->FullCategory;
		Value->Graphs = OriginalDataSource.FilterByPredicate([&SearchCategory](const TSharedPtr<FInstaMATGraphObjectViewItem>& GraphItem)
		{
			return SearchCategory.Equals(GraphItem->Category, ESearchCase::IgnoreCase);
		});
	}

	// The fnRetrieveGraphObjectsOfChildrenRecursive lambda adds the child graphs to their parents.
	TFunction<TArray<TSharedPtr<FInstaMATGraphObjectViewItem>>& (const TSharedPtr<FCategoryHierarchyNode>&)> fnRetrieveGraphObjectsOfChildrenRecursive;
	fnRetrieveGraphObjectsOfChildrenRecursive = [&fnRetrieveGraphObjectsOfChildrenRecursive](const TSharedPtr<FCategoryHierarchyNode>& Item) -> TArray<TSharedPtr<FInstaMATGraphObjectViewItem>>&
	{
		for (const auto& Child : Item->Children)
		{
			const TArray<TSharedPtr<FInstaMATGraphObjectViewItem>>& ChildGraphs = fnRetrieveGraphObjectsOfChildrenRecursive(Child);

			for (const TSharedPtr<FInstaMATGraphObjectViewItem>& ChildGraph : ChildGraphs)
			{
				Item->Graphs.AddUnique(ChildGraph);
			}
		}
		return Item->Graphs;
	};

	// Ensure that all category entries have their graphs loaded
	for (TSharedPtr<FCategoryHierarchyNode>& RootNode : OriginalCategoryRoots)
	{
		fnRetrieveGraphObjectsOfChildrenRecursive(RootNode);
	}

	// Remove categories that have empty graphs
	OriginalCategoryRoots.RemoveAll([](const TSharedPtr<FCategoryHierarchyNode>& RootNode) { return RootNode->Graphs.Num() == 0; });

	const float kPadding = 5.0f;
	const float kVerticalBoxMaximumHeight = 48.0f; 
	const float kItemSlotSizePercentage = 0.35f;
	const float kSplitterHandleSize = 3.0f;
	const float kMinimumSplitterSize = 140.0f;

	ChildSlot
		[
			SNew(SOverlay)
				+ SOverlay::Slot()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					SNew(SImage)
						.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Background")))
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Fill)
						.MaxHeight(kVerticalBoxMaximumHeight)
						[
							SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SImage)
										.Image(FInstaMATPluginStyle::Get().GetBrush("InstaMATUI.BackgroundGray.Brush"))
								]
								+ SOverlay::Slot()
								.Padding(kPadding * 2.0f, kPadding * 2.0f)
								[
									// InstaMAT Logo
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.HAlign(HAlign_Left)
										[
											SNew(SScaleBox)
												[
													SNew(SImage)
														.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMAT.LogoTiny")))
												]
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.HAlign(HAlign_Right)
										[
											SAssignNew(HelpButton, SButton)
												.ButtonStyle(&FInstaMATPluginStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("InstaMATUI.HelpButton")))
												.OnClicked(this, &SInstaMATGraphLibraryWindow::OnClickedHelpButtonHandler)
												.Content()
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot()
														.HAlign(HAlign_Left)
														.Padding(kPadding)
														[
															SNew(SScaleBox)
																.Stretch(EStretch::None)
																[
																	SNew(SImage)
																		.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Icons.Info")))
																]
														]
														+ SHorizontalBox::Slot()
														.Padding(kPadding)
														.AutoWidth()
														.VAlign(VAlign_Center)
														.HAlign(HAlign_Left)
														[
															SNew(STextBlock)
																.TextStyle(FInstaMATPluginStyle::Get(), TEXT("InstaMAT.Bold"))
																.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "Library_help_button", "Get Help or Join the Community"))
														]
												]
										]
								]
						]
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.Padding(0.0f, 0.0f, 0.0f, 0.0f)
						[
							SAssignNew(LibraryWindowContentLayout, SVerticalBox)
						]
				]
		];

	const bool bIsHostAuthorized = InstaMATInterface->GetInstaMAT()->IsHostAuthorized();

	if (bIsHostAuthorized && !InstaMATModule.bIsInstaMATFloatingLicenseAvailable)
	{
		InstaMATModule.bIsInstaMATFloatingLicenseAvailable = true;
		InstaMATModule.ForceLicenseRefreshDelegate = nullptr;
	}

	// In case the machine is not authorized we only show an error message
	if (!bIsHostAuthorized)
	{
		static const float kErrorPadding = 20.0f;
		LibraryWindowContentLayout->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(0.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.Padding(kErrorPadding)
			[
				// Info text
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("InstaMAT is not authorized. Please authorize the workstation through the settings menu.")))
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FSlateColor(FLinearColor(FColor::Red)))
			]
		];
		return;
	}

	LibraryWindowContentLayout->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(0.0f, 0.0f, 0.0f, 0.0f)
		[
			SAssignNew(LeftSplitter, SSplitter)
			.Clipping(EWidgetClipping::ClipToBoundsAlways)
			.OnSplitterFinishedResizing(this, &SInstaMATGraphLibraryWindow::OnSplitterResizeFinished)
			.Orientation(EOrientation::Orient_Horizontal)
			.PhysicalSplitterHandleSize(kSplitterHandleSize)
			.MinimumSlotHeight(kMinimumSplitterSize)
						
			.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FSplitterStyle>(TEXT("InstaMATUI.Splitter")))
			+ SSplitter::Slot()
			.MinSize(kMinimumSplitterSize)
			.Value(/*DefaultSize:*/ 0.30f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.AutoHeight()
				[
					SAssignNew(SearchField, SSearchBox)
					.OnTextChanged(this, &SInstaMATGraphLibraryWindow::OnSearchTextChanged)
				]
				+SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				[
					SNew(SSplitter)
						.Clipping(EWidgetClipping::ClipToBoundsWithoutIntersecting)
						.Orientation(EOrientation::Orient_Vertical)
						.PhysicalSplitterHandleSize(kSplitterHandleSize)
						.MinimumSlotHeight(0.0f)
						.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FSplitterStyle>(TEXT("InstaMATUI.Splitter")))
						+SSplitter::Slot()
						.Expose(CategoriesSplitterSlot)
						.MinSize(kMinimumSplitterSize)
						[
							SAssignNew(CategoriesSplitterSlotContent, SHorizontalBox)
							+SHorizontalBox::Slot()
							.VAlign(VAlign_Fill)
							.AutoWidth()
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									SNew(SImage)
										.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.Background.Brush.Dark")))
								]
								+ SOverlay::Slot()
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.VAlign(VAlign_Top)
									.AutoHeight()
									[
										SNew(SInstaMATSegmentedControl)
										.ElementsInformation(TArray<FInstaMATSegmentedControlElement>({
										FInstaMATSegmentedControlElement(
											FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Materials.Normal")),
											FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Materials.Hovered")),
											NSLOCTEXT(LOCTEXT_NAMESPACE, "Materials", "Materials")),
										FInstaMATSegmentedControlElement(
											FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Mesh.Normal")),
											FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Mesh.Hovered")),
											NSLOCTEXT(LOCTEXT_NAMESPACE, "Mesh", "Mesh")),
										FInstaMATSegmentedControlElement(
											FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.User.Normal")),
											FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.User.Hovered")),
											NSLOCTEXT(LOCTEXT_NAMESPACE, "User", "User")),
										FInstaMATSegmentedControlElement(
											FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.NoFilter.Normal")),
											FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.NoFilter.Hovered")),
											NSLOCTEXT(LOCTEXT_NAMESPACE, "All", "All"))
											}))
										.ActiveELementIndex(uint64(Filter))
										.OnActiveButtonChanged_Lambda([this](const uint64 Index) -> void {OnFilterSelectionChanged((EInstaMATLibraryFilter)Index); })
									]
								]
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SBox)
								.Visibility_Lambda([this]() -> EVisibility {return CategoryView->HasAnyUserFocus() ? EVisibility::Visible : EVisibility::Hidden; })
								.WidthOverride(1.0f)
								[
									SNew(SImage)
										.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.BlueBorder")))
								]
							]
							+ SHorizontalBox::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.VAlign(VAlign_Fill)
								[
									SNew(SBox)
									.Padding(0.0f, kPadding, 0.0f, 0.0f)
									[
										// Category
										SAssignNew(CategoryView, STreeView<TSharedPtr<FCategoryHierarchyNode>>)
										.ScrollBarStyle(&FInstaMATPluginStyle::Get().GetWidgetStyle<FScrollBarStyle>(TEXT("InstaMATUI.Scrollbar")))
										.SelectionMode(ESelectionMode::Single)
										.TreeViewStyle(&FInstaMATPluginStyle::Get().GetWidgetStyle<FTableViewStyle>(TEXT("InstaMATUI.Browser.Category")))
										.TreeItemsSource(&OriginalCategoryRoots)
										.OnGenerateRow_Lambda(fnGenerateCategoryViewRow)
										.OnSelectionChanged(this, &SInstaMATGraphLibraryWindow::OnCategorySelectionChanged)
										.OnGetChildren_Lambda([](TSharedPtr<FCategoryHierarchyNode> InItem, TArray<TSharedPtr<FCategoryHierarchyNode>>& OutChildren)
											{
												OutChildren.Append(InItem->Children);
											})
									]
								]
							]
						]
						+ SSplitter::Slot()
						.MinSize(kMinimumSplitterSize)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Fill)
								.AutoWidth()
								[
									SNew(SOverlay)
									+SOverlay::Slot()
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									[
										SNew(SImage)
										.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.Background.Brush.Dark")))
									]
									+SOverlay::Slot()
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.VAlign(VAlign_Top)
										.AutoHeight()
										[
											SNew(SInstaMATSideBarButton)
											.OnButtonToggled(this, &SInstaMATGraphLibraryWindow::OnSearchInSelectedCategoryChanged)
											.State(ECheckBoxState::Unchecked)
											.IconHovered(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Directory.Hovered")))
											.IconNormal(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Directory.Normal")))
											.ToolTipText(NSLOCTEXT(LOCTEXT_NAMESPACE, "SearchInSelectedCategory", "Search In Selected Category"))
										]
										+ SVerticalBox::Slot()
										.Padding(0.0f, 4.0f)
										.AutoHeight()
										[
											SNew(SBox)
											.Padding(4.0f, 0.0f)
											.HeightOverride(1.0f)
											.WidthOverride(20.0f)
											[
												SNew(SImage)
												.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.Separator.Brush")))
											]
										]
										+SVerticalBox::Slot()
										.VAlign(VAlign_Top)
										.AutoHeight()
										[
											SNew(SInstaMATSegmentedControl)
											.ElementsInformation(TArray<FInstaMATSegmentedControlElement>({
											FInstaMATSegmentedControlElement(
												FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Table.Normal")),
												FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Table.Hovered")),
												NSLOCTEXT(LOCTEXT_NAMESPACE, "UseATableView", "Use A Table View")),
											FInstaMATSegmentedControlElement(
												FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.RowIcon.Normal")),
												FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.RowIcon.Hovered")),
												NSLOCTEXT(LOCTEXT_NAMESPACE, "DisplaySmallIcons", "Display Small Icons")),
											FInstaMATSegmentedControlElement(
												FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Tiles.Normal")),
												FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.Browser.Tiles.Hovered")),
												NSLOCTEXT(LOCTEXT_NAMESPACE, "DisplayLargeIcons", "Display Large Icons")),
											}))
											.ActiveELementIndex(uint64(RowType))
											.OnActiveButtonChanged_Lambda([this](const uint64 Index) {OnGraphViewTypeChanged((EInstaMATLibraryGraphListRowType)Index); })
										]
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SBox)
									.Visibility_Lambda([this]() -> EVisibility {return GraphView->HasAnyUserFocus() ? EVisibility::Visible : EVisibility::Hidden; })
									.WidthOverride(1.0f)
									[
										SNew(SImage)
										.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SideBar.BlueBorder")))
									]
								]
								+ SHorizontalBox::Slot()
								.HAlign(HAlign_Fill)
								[
									SAssignNew(GraphView, STileView<TSharedPtr<FInstaMATGraphObjectViewItem>>)
									.ScrollBarStyle(&FInstaMATPluginStyle::Get().GetWidgetStyle<FScrollBarStyle>("InstaMATUI.Scrollbar"))
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
									.ScrollbarDisabledVisibility(EVisibility::Hidden)
#endif
									.ListItemsSource(&GraphDataSource)
									.SelectionMode(ESelectionMode::Single)
									.OnGenerateTile(this, &SInstaMATGraphLibraryWindow::GenerateGraphItemRow)
									.OnMouseButtonDoubleClick_Lambda(fnOnItemDoubleClicked)
									.OnSelectionChanged(this, &SInstaMATGraphLibraryWindow::OnGraphSelectionChanged)
								]
						]
					]
			]
			// Preview widget
			+ SSplitter::Slot()
			.MinSize(kMinimumSplitterSize)
			[
				SAssignNew(PreviewWidget, SInstaMATGraphLibraryPreviewWidget)
			]
		];

	OnGraphViewTypeChanged(RowType);
	OnFilterSelectionChanged(EInstaMATLibraryFilter::InstaMAT_Material);
}

void SInstaMATGraphLibraryWindow::OnFilterSelectionChanged(const EInstaMATLibraryFilter NewFilter)
{
	CategoryView->ClearSelection();
	Filter = NewFilter;
	TArray<TSharedPtr<FCategoryHierarchyNode>>& Source = OriginalCategoryRoots;
	CategoryRootsSource.Reset();

	// Clean search text
	if (!SearchField->GetText().IsEmpty())
	{
		SearchField->SetText(FText());
	}

	/// The fnFilterByString filters the categories by the specified Value.
	const auto fnFilterByString = [&Source](const FString& Value, TArray<TSharedPtr<FCategoryHierarchyNode>>& OutList)
	{
		for (const TSharedPtr<FCategoryHierarchyNode>& Item : Source)
		{
			if (Item->Category.Compare(Value, ESearchCase::IgnoreCase) == 0)
			{
				OutList.Add(Item);
			}
		}
	};

	const uint32 kMaximumDepth = 512U;

	/// The fnFilterByCategoriesArray filters the categories based on the FilterCategories array.
	TFunction<void(const TArray<TSharedPtr<FCategoryHierarchyNode>>& , const TArray<FString>&, TArray<TSharedPtr<FCategoryHierarchyNode>>&, int depth)> fnFilterByCategoriesArrayRecursive;
	fnFilterByCategoriesArrayRecursive = [&fnFilterByCategoriesArrayRecursive](const TArray<TSharedPtr<FCategoryHierarchyNode>>& Source, const TArray<FString>& FilterCategories, TArray<TSharedPtr<FCategoryHierarchyNode>>& OutList, const uint32 depth)
	{
		for (const TSharedPtr<FCategoryHierarchyNode>& Item : Source)
		{
			const bool bContainsPredicate = FilterCategories.ContainsByPredicate([&Item](const FString& Value)
			{
				return Item->FullCategory.Compare(Value, ESearchCase::IgnoreCase) == 0;
			});

			TArray<TSharedPtr<FCategoryHierarchyNode>> TempArray;
			
			if (depth < kMaximumDepth)
			{
				fnFilterByCategoriesArrayRecursive(Item->Children, FilterCategories, TempArray, depth + 1u);
			}

			if (!bContainsPredicate && TempArray.Num() == 0)
			{
				Item->bIsVisible = false;
				continue;
			}

			Item->bIsVisible = true;
			OutList.AddUnique(Item);
		}
	};

	/// The fnEnsureVisibilityForUserCategoriesRecursive ensures correct visiblity state for the category items in the user filter.
	TFunction<bool(const TArray<TSharedPtr<FCategoryHierarchyNode>>&, const uint32)> fnEnsureVisibilityForUserCategoriesRecursive;
	fnEnsureVisibilityForUserCategoriesRecursive = [&fnEnsureVisibilityForUserCategoriesRecursive](const TArray<TSharedPtr<FCategoryHierarchyNode>>& Source, const uint32 depth) -> bool
		{
			bool bHasItemUserGraphs = false;

			for (const TSharedPtr<FCategoryHierarchyNode>& Item : Source)
			{
				bool bContainsUserGraphs = false;

				for (const TSharedPtr<FInstaMATGraphObjectViewItem>& Graph : Item->Graphs)
				{
					if (!Graph->bIsUserGraph)
						continue;

					bContainsUserGraphs = true;
					break;
				}

				if (depth < kMaximumDepth)
				{
					if (fnEnsureVisibilityForUserCategoriesRecursive(Item->Children, depth + 1u))
					{
						bContainsUserGraphs = true;
					}
				}

				if (!bContainsUserGraphs)
				{
					Item->bIsVisible = false;
					continue;
				}

				bHasItemUserGraphs = true;
				Item->bIsVisible = true;
			}

			return bHasItemUserGraphs;
		};

	// Based on the Filter the data sources for the CategoryView must be updated
	switch (Filter)
	{
	case EInstaMATLibraryFilter::InstaMAT_Material:
		if (MaterialFilteredCategories.Num() == 0)
		{
			fnFilterByString(TEXT("Materials"), MaterialFilteredCategories);
		}
		CategoryRootsSource = MaterialFilteredCategories;
		CategoryView->SetTreeItemsSource(&MaterialFilteredCategories);
		break;
	case EInstaMATLibraryFilter::InstaMAT_All:
		CategoryRootsSource = OriginalCategoryRoots;
		CategoryView->SetTreeItemsSource(&OriginalCategoryRoots);
		break;
	case EInstaMATLibraryFilter::InstaMAT_Mesh:
		if (MeshFilteredCategories.Num() == 0)
		{
			fnFilterByString(TEXT("Mesh"), MeshFilteredCategories);
		}
		CategoryRootsSource = MeshFilteredCategories;
		CategoryView->SetTreeItemsSource(&MeshFilteredCategories);
		break;
	case EInstaMATLibraryFilter::InstaMAT_User:
	{
		if (UserCategories.Num() == 0)
		{
			FInstaMATModule& InstaMATModule = FModuleManager::GetModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
			IInstaMAT* const InstaMATInterface = InstaMATModule.GetInstaMATInterface();
			check(InstaMATInterface != nullptr);

			const TArray<FString>& PrivateCategories = InstaMATInterface->GetUserCategories(/*bEnforceUpdate:*/ true);

			fnFilterByCategoriesArrayRecursive(Source, PrivateCategories, UserCategories, 0u);
		}
		else
		{
			fnEnsureVisibilityForUserCategoriesRecursive(UserCategories, 0u);
		}

		CategoryRootsSource = UserCategories;
		CategoryView->SetTreeItemsSource(&UserCategories);
	}
		break;
	default:
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Invalid filter."));
		break;
	}

	// Ensure all items from the current data source are visible
	for (const TSharedPtr<FCategoryHierarchyNode>& Item : CategoryRootsSource)
	{
		CategoryView->SetItemExpansion(Item, /*InShouldExpandItem:*/ true);

		// NOTE: The visibility for the items in the user filter is handled in the fnFilterByCategoriesArrayRecursive/fnEnsureVisibilityForUserCategoriesRecursive to make children invisible that are not part of the user categories
		if (Filter != EInstaMATLibraryFilter::InstaMAT_User)
		{
			InstaMATGraphLibraryWindowUtilities::SetCategoryItemsVisibilityStatusRecursive(Item, true, 0u);
		}
	}

	// Enables the category filter, these are categories that should be invisible to a user
	if (Filter != EInstaMATLibraryFilter::InstaMAT_User)
	{
		InstaMATGraphLibraryWindowUtilities::SetVisibilityBasedOnFilterArray(CategoryRootsSource);
	}

	CategoryView->RebuildList();

	// Clean data source for the graph view 
	GraphDataSource.Empty();
	GraphView->RebuildList();
}

void SInstaMATGraphLibraryWindow::SortDataSource(TArray<TSharedPtr<FInstaMATGraphObjectViewItem>>& DataSource)
{
	DataSource.Sort([](const TSharedPtr<FInstaMATGraphObjectViewItem>& LHS, const TSharedPtr<FInstaMATGraphObjectViewItem>& RHS)
	{
		return LHS->GraphFriendlyName.Compare(RHS->GraphFriendlyName, ESearchCase::IgnoreCase) < 0;
	});
}

void SInstaMATGraphLibraryWindow::OnGraphViewTypeChanged(const EInstaMATLibraryGraphListRowType Type)
{
	check(GraphView != nullptr);

	RowType = Type;

	const float ControlWidth = GetSplitterFirstSlotWidth();

	static const float kTileBigWidth = 135.0f;
	static const float kTileBigHeight = 200.0f;

	float RowHeight = kTileBigHeight;
	float RowWidth = kTileBigWidth;

	// Determine the Row height based on the RowType
	if (Type == EInstaMATLibraryGraphListRowType::InstaMAT_ListSmallIcon)
	{
		RowHeight = 24.0f;
		RowWidth = 160.0f;
	}
	else if (Type == EInstaMATLibraryGraphListRowType::InstaMAT_TileMediumIcon)
	{
		RowHeight = 150.0f;
		RowWidth = 100.0f;
	}

	GraphView->SetItemHeight(RowHeight);
	GraphView->SetItemWidth(RowWidth);

	GraphView->RebuildList();
}

void SInstaMATGraphLibraryWindow::OnSearchInSelectedCategoryChanged(const ECheckBoxState NewState)
{
	check(SearchField != nullptr);

	bIsSearchInSelectedCategoryActive = NewState == ECheckBoxState::Checked;

	if(SearchField->GetText().IsEmpty())
		return;

	OnSearchTextChanged(SearchField->GetText());
}

void SInstaMATGraphLibraryWindow::SetCategoriesUIVisibility(const bool bIsVisible)
{
	check(CategoriesSplitterSlot)

	if (CategoriesSplitterSlotContent->GetVisibility() == EVisibility::Visible && bIsVisible)
		return;

	static float SizeValue = 0.0f;
	static float MinimumSize = -1.0f;
	if (MinimumSize < 0)
	{
		MinimumSize = CategoriesSplitterSlot->GetMinSize();
	}

	if (!bIsVisible)
	{
		SizeValue = CategoriesSplitterSlot->GetSizeValue();
	}

	CategoriesSplitterSlot->SetSizeValue(bIsVisible? SizeValue : 0.0f);
	CategoriesSplitterSlot->SetMinSize(MinimumSize);
	CategoriesSplitterSlot->SetResizable(bIsVisible);

	CategoriesSplitterSlotContent->SetVisibility(bIsVisible? EVisibility::Visible : EVisibility::Collapsed);
}

TSharedRef<ITableRow> SInstaMATGraphLibraryWindow::GenerateGraphItemRow(TSharedPtr<FInstaMATGraphObjectViewItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	check(Item != nullptr);
	
	const float kPadding = 5.0f;
	// Load Preview image if available
	if (Item->Preview == nullptr && FInstaMATImporterUtility::IsCachedPreviewImageAvailable(Item->GraphID))
	{
		FInstaMATImporterUtility::LoadPreviewTexture(*Item.Get());
	}

	static const FVector2D kBigImageBrushSize = FVector2D(120.0f, 120.0f);
	FVector2D ImageBrushSize = kBigImageBrushSize;

	// Determine the rendered icon size based on the RowType
	if (RowType == EInstaMATLibraryGraphListRowType::InstaMAT_ListSmallIcon)
	{
		ImageBrushSize = FVector2D(16.0f, 16.0f);
	}
	else if (RowType == EInstaMATLibraryGraphListRowType::InstaMAT_TileMediumIcon)
	{
		ImageBrushSize = FVector2D(80.0f, 80.0f);
	}
	
	if (Item->GraphSelectionBrush == nullptr && Item->Preview != nullptr)
	{
		Item->GraphSelectionBrush = MakeShared<FSlateDynamicImageBrush>(Item->Preview, kBigImageBrushSize, FName(TEXT("InstaMATPreview")));
	}

	// Draws a simple row item: Icon small left - Graph name right
	if (RowType == EInstaMATLibraryGraphListRowType::InstaMAT_ListSmallIcon)
	{
		return
			SNew(STableRow<TSharedPtr<FInstaMATGraphObjectViewItem>>, OwnerTable)
			.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("InstaMATUI.Browser.Tiles")))
			.Padding(kPadding)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, kPadding, 0.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Left)
				[
					SNew(SScaleBox)
					.OverrideScreenSize(ImageBrushSize)
					.Stretch(EStretch::None)
					[
						SNew(SImage)
						.DesiredSizeOverride(ImageBrushSize)
						.Image(Item->GraphSelectionBrush == nullptr ? FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.SmallIcon")) : Item->GraphSelectionBrush.Get())
					]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
					.Justification(ETextJustify::Left)
					.Text(FText::FromString(Item->GraphFriendlyName))
				]
			];
	}

	// Draws a tile: Big icon top centered - Graph name middle centered - Graph category bottom centered
	return
		SNew(STableRow<TSharedPtr<FInstaMATGraphObjectViewItem>>, OwnerTable)
		.Style(&FInstaMATPluginStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("InstaMATUI.Browser.Tiles")))
		.Padding(kPadding)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::None)
				[
					SNew(SImage)
					.DesiredSizeOverride(ImageBrushSize)
					.Image(Item->GraphSelectionBrush == nullptr ? FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.MediumIcon")) : Item->GraphSelectionBrush.Get())
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Center)
			.Padding(0.0f, kPadding / 2.f, 0.0f, kPadding / 2.f)
			[
				SNew(SBox)
					.MaxDesiredHeight(30.0f)
					[
						SNew(STextBlock)
							.AutoWrapText(true)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
							.Justification(ETextJustify::Center)
							.Text(FText::FromString(Item->GraphFriendlyName))
					]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Center)
			[
				SNew(SBox)
					.MaxDesiredHeight(23.0f)
					[
						SNew(STextBlock)
							.AutoWrapText(true)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.TextStyle(FInstaMATPluginStyle::Get(), TEXT("InstaMATUI.Library.Small"))
							.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
							.Justification(ETextJustify::Center)
							.Text(FText::FromString(Item->Category))
					]
			]
		];
}

void SInstaMATGraphLibraryWindow::OnSplitterResizeFinished()
{
	OnGraphViewTypeChanged(RowType);
}

float SInstaMATGraphLibraryWindow::GetSplitterFirstSlotWidth()
{
	const FChildren* const Children = LeftSplitter->GetChildren();

	if (Children == nullptr)
		return 128.0f;

	const FGeometry& Geometry = Children->GetSlotAt(0).GetWidget()->GetCachedGeometry();
	return FMath::Max(Geometry.Size.X - 55.0f, 50.0f);
}

FReply SInstaMATGraphLibraryWindow::OnClickedHelpButtonHandler()
{
	check(HelpButton.IsValid());
	
	// Calculate spawn position
	const FGeometry& Geometry = HelpButton->GetCachedGeometry();
	const float kVerticalOffset = 10.0f;
	const FVector2D SpawnPosition = Geometry.GetAbsolutePosition() + FVector2D(Geometry.GetAbsoluteSize().X / 2.0f - SInstaMATContextMenu::GetApproximateWidth() / 2.0f, Geometry.GetAbsoluteSize().Y + kVerticalOffset);

	TSharedRef<SInstaMATContextMenu> ContextMenu = SNew(SInstaMATContextMenu);

	/// The fnOnClickedHandler macro creates a click handler that opens the specified website.
#define fnOnClickedHandler(X)														\
FOnClicked::CreateLambda([/*copy:*/ ContextMenu]() -> FReply {						\
	FPlatformProcess::LaunchURL((X), /*Parms:*/ nullptr, /*Error:*/ nullptr);		\
	FSlateApplication::Get().DismissMenuByWidget(ContextMenu);						\
	return FReply::Handled();														\
})

	// Add entries
	const ISlateStyle& Style = FInstaMATPluginStyle::Get();
	ContextMenu->AddEntry(Style.GetBrush(TEXT("InstaMATUI.Icons.Speech")), NSLOCTEXT(LOCTEXT_NAMESPACE, "Help_AskQuestion", "Ask questions and get answers in the Abstract Community"), fnOnClickedHandler(TEXT("https://community.TheAbstract.co/")));
	ContextMenu->AddEntry(Style.GetBrush(TEXT("InstaMATUI.Icons.Youtube")), NSLOCTEXT(LOCTEXT_NAMESPACE, "Help_Youtube", "Watch tutorials and subscribe to our YouTube Channel"), fnOnClickedHandler(TEXT("https://www.youtube.com/@InstaMAT_io")));
	ContextMenu->AddEntry(Style.GetBrush(TEXT("InstaMATUI.Icons.Download")), NSLOCTEXT(LOCTEXT_NAMESPACE, "Help_Download", "Download the latest version of InstaMAT products"), fnOnClickedHandler(TEXT("https://cloud.InstaMAT.io")));
	ContextMenu->AddEntry(Style.GetBrush(TEXT("InstaMATUI.Icons.Discord")), NSLOCTEXT(LOCTEXT_NAMESPACE, "Help_Discord", "Join our Discord Server"), fnOnClickedHandler(TEXT("https://community.TheAbstract.co/t/instamat-discord-channel/401")));
	ContextMenu->AddEntry(Style.GetBrush(TEXT("InstaMATUI.Icons.X")), NSLOCTEXT(LOCTEXT_NAMESPACE, "Help_Twitter", "Follow us on X / Twitter"), fnOnClickedHandler(TEXT("https://twitter.com/InstaMAT_io")));
	ContextMenu->AddEntry(Style.GetBrush(TEXT("InstaMATUI.Icons.Book")), NSLOCTEXT(LOCTEXT_NAMESPACE, "Help_NodeReference", "Explore nodes in the Library Reference"), fnOnClickedHandler(TEXT("https://node.docs.InstaMAT.io/latest/index.html")));
	ContextMenu->AddEntry(Style.GetBrush(TEXT("InstaMATUI.Icons.Question")), NSLOCTEXT(LOCTEXT_NAMESPACE, "Help_KnowledgeBase", "Find answers in the Knowledge Base"), fnOnClickedHandler(TEXT("https://docs.InstaMAT.io/en/KnowledgeBase")));
	ContextMenu->AddEntry(Style.GetBrush(TEXT("InstaMATUI.Icons.World")), NSLOCTEXT(LOCTEXT_NAMESPACE, "Help_Website", "Visit the InstaMAT website"), fnOnClickedHandler(TEXT("https://www.InstaMaterial.com/")));

#undef fnOnClickedHandler

	FSlateApplication::Get().PushMenu(
		AsShared(),
		FWidgetPath(),
		ContextMenu,
		SpawnPosition,
		FPopupTransitionEffect(FPopupTransitionEffect::TopMenu));

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
