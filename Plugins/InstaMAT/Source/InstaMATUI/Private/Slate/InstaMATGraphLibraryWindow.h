/**
 * InstaMATGraphLibraryWindow.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATGraphLibraryWindow.h
 * @copyright 2019-2021 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_InstaMATGraphLibraryWindow_h
#define InstaMAT_InstaMATGraphLibraryWindow_h

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "InstaMATGraphLibraryPreviewWidget.h"
#include "InstaMATModule.h"

/**
 * The FCategoryHierarchyNode is the data representation for the category viewer hierarchy.
 */
struct FCategoryHierarchyNode
{
	FString FullCategory;										/**< The full category path. */	
	FString Category;											/**< The category. */
	TArray<TSharedPtr<FCategoryHierarchyNode>> Children;		/**< The children of this category. */
	TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> Graphs;	/**< The Graph representation. */
	bool bIsVisible;											/**< Whether the item is visible. */

	/**
	 * Clears the instance.
	 */
	void Empty()
	{
		Children.Empty();
		Graphs.Empty();
	}
};

/**
 * The SInstaMATGraphLibraryWindow is placed inside of a dockable tab.
 * It shows all registered graphs.
 */
class SInstaMATGraphLibraryWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInstaMATGraphLibraryWindow) {}
	SLATE_ATTRIBUTE(TArray<TSharedPtr<FInstaMATGraphObjectViewItem>>, GraphObjects)
	SLATE_END_ARGS()

	SInstaMATGraphLibraryWindow();
	~SInstaMATGraphLibraryWindow();

	/**
	 * Constructs the view of this instance.
	 *
	 * @param InArgs the construction arguments.
	 */
	void Construct(const FArguments& InArgs);

private:

	/**
	 * The category selection changed handler.
	 * 
	 * @param SelectedNode the selected node.
	 * @param SelectionType the selection type.
	 */
	void OnCategorySelectionChanged(TSharedPtr<FCategoryHierarchyNode> SelectedNode, ESelectInfo::Type SelectionType);
	
	/**
	 * The graph selection changed handler.
	 *
	 * @param SelectedNode the selected node.
	 * @param SelectionType the selection type.
	 */
	void OnGraphSelectionChanged(TSharedPtr<FInstaMATGraphObjectViewItem> SelectedNode, ESelectInfo::Type SelectionType);

	/**
	 * Sets the filter of this instance.
	 *
	 * @param NewFilter the new filter.
	 */
	void OnFilterSelectionChanged(const EInstaMATLibraryFilter NewFilter);

	/**
	 * The search text changed handler.
	 * 
	 * @param Text The search text;
	 */
	void OnSearchTextChanged(const FText& Text);

	/**
	 * The graph view type changed handler.
	 * 
	 * @param Type The row type.
	 */
	void OnGraphViewTypeChanged(const EInstaMATLibraryGraphListRowType Type);

	/**
	 * The search in selected category changed handler.
	 *
	 * @param NewState The new state of search in selected category button.
	 */
	void OnSearchInSelectedCategoryChanged(const ECheckBoxState NewState);

	/**
	* Sets the visibility of the categories UI. Used to change the visibility when the global/local search becomes active/inactive.
	* 
	* @param bIsVisible The new visibility state of the categories UI.
	*/
	void SetCategoriesUIVisibility(const bool bIsVisible);

	/**
	 * Generates a graph item row for the GraphView.
	 * @note based on the current RowType a different
	 * layout is generated.
	 * 
	 * @param Item The item.
	 * @param Owner Table the owner table.
	 * @return The generated row.
	 */
	TSharedRef<ITableRow> GenerateGraphItemRow(TSharedPtr<FInstaMATGraphObjectViewItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * Called when the LeftSplitter resize finished.
	 */
	void OnSplitterResizeFinished();

	/**
	 * Gets the width of the first splatter slot.
	 */
	float GetSplitterFirstSlotWidth();

	/**
	 * Help button click handler.
	 * 
	 * @return The reply state.
	 */
	FReply OnClickedHelpButtonHandler();

	/**
	 * Sorts the specified data source.
	 * 
	 * @param DataSource the data source to sort.
	 */
	static void SortDataSource(TArray<TSharedPtr<FInstaMATGraphObjectViewItem>>& DataSource);

	TArray<TSharedPtr<FCategoryHierarchyNode>> MaterialFilteredCategories;		/**< All category options that are visible when the Default filter is enabled. */
	TArray<TSharedPtr<FCategoryHierarchyNode>> MeshFilteredCategories;			/**< All category options that are visible when the Mesh filter is enabled. */
	TArray<TSharedPtr<FCategoryHierarchyNode>> CategoryRoots;					/**< The data source for the category hierarchy. */
	TArray<TSharedPtr<FCategoryHierarchyNode>> CategoryRootsSource;				/**< The unfiltered source for the category hierarchy. */
	TArray<TSharedPtr<FCategoryHierarchyNode>> OriginalCategoryRoots;			/**< Unfiltered category hierarchy. */
	TArray<TSharedPtr<FCategoryHierarchyNode>> UserCategories;					/**< All category options that are visible when the user filter is enabled. */
	TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> GraphDataSource;			/**< The data source for the list. Based on this array the rows are generated. */
	TArray<TSharedPtr<FInstaMATGraphObjectViewItem>> OriginalDataSource;		/**< Unfiltered original source. */
	TSharedPtr<SSplitter> LeftSplitter;											/**< The splitter between browsers and preview widget. */
	TSharedPtr<SSearchBox> SearchField;											/**< The search field. */
	TSharedPtr<SVerticalBox> LibraryWindowContentLayout;						/**< The layout of the library window content. */
	TSharedPtr<STreeView<TSharedPtr<FCategoryHierarchyNode>>> CategoryView;		/**< The hierarchy view for the category. */
	TSharedPtr<SListView<TSharedPtr<FInstaMATGraphObjectViewItem>>> GraphView;	/**< The list view of this widget. */
	TSharedPtr<FCategoryHierarchyNode> LastSelectedCategory;					/**< The last selected category. */
	TSharedPtr<SInstaMATGraphLibraryPreviewWidget> PreviewWidget;				/**< The preview widget. */
	TSharedPtr<SButton> HelpButton;												/**< The help button. */
	TSharedPtr<SHorizontalBox> CategoriesSplitterSlotContent;					/**< The content of the categories slot of the left side splitter .*/
	SSplitter::FSlot* CategoriesSplitterSlot;									/**< The categories slot of the left side splitter. */
	EInstaMATLibraryFilter Filter;												/**< The Library filter. */
	EInstaMATLibraryGraphListRowType RowType;									/**< The graph row type. */
	bool bIsSearchInSelectedCategoryActive;										/**< Whether the search in selected category button is active. */
};

#endif
