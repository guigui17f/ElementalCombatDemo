#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "ISMMergeTypes.h"

class UISMMergeTool;
class SCheckBox;
class STextBlock;
class SButton;

DECLARE_DELEGATE_OneParam(FOnISMActorSelectionChanged, bool);
DECLARE_DELEGATE_OneParam(FOnStaticMeshActorSelectionChanged, bool);

/**
 * ISM Actor列表项控件
 */
class SISMActorItem : public STableRow<TSharedPtr<FISMActorData>>
{
public:
	SLATE_BEGIN_ARGS(SISMActorItem) {}
		SLATE_ARGUMENT(TSharedPtr<FISMActorData>, ActorData)
		SLATE_EVENT(FOnISMActorSelectionChanged, OnSelectionChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

private:
	ECheckBoxState IsChecked() const;
	void OnCheckStateChanged(ECheckBoxState NewState);
	FText GetActorDisplayText() const;
	FText GetISMComponentCountText() const;

private:
	TSharedPtr<FISMActorData> ActorData;
	FOnISMActorSelectionChanged OnSelectionChanged;
};

/**
 * 静态网格体Actor列表项控件
 */
class SStaticMeshActorItem : public STableRow<TSharedPtr<FStaticMeshActorData>>
{
public:
	SLATE_BEGIN_ARGS(SStaticMeshActorItem) {}
		SLATE_ARGUMENT(TSharedPtr<FStaticMeshActorData>, ActorData)
		SLATE_EVENT(FOnStaticMeshActorSelectionChanged, OnSelectionChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

private:
	ECheckBoxState IsChecked() const;
	void OnCheckStateChanged(ECheckBoxState NewState);
	FText GetActorDisplayText() const;
	FText GetStaticMeshText() const;
	FText GetCompatibilityText() const;
	FSlateColor GetCompatibilityTextColor() const;

private:
	TSharedPtr<FStaticMeshActorData> ActorData;
	FOnStaticMeshActorSelectionChanged OnSelectionChanged;
};

/**
 * 主ISM合并界面控件
 */
class ELEMENTALCOMBATEDITOR_API SISMMergeWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SISMMergeWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** Refresh actor lists from current selection */
	void RefreshActorLists();

	/** Generate ISM Actor List Widget */
	TSharedRef<ITableRow> OnGenerateISMActorRow(TSharedPtr<FISMActorData> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Generate Static Mesh Actor List Widget */
	TSharedRef<ITableRow> OnGenerateStaticMeshActorRow(TSharedPtr<FStaticMeshActorData> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Handle ISM actor selection change */
	void OnISMActorSelectionChanged(bool bIsSelected);

	/** Handle static mesh actor selection change */
	void OnStaticMeshActorSelectionChanged(bool bIsSelected);

	/** Update compatibility results for static mesh actors */
	void UpdateCompatibilityResults();

	/** Get settings widget */
	TSharedRef<SWidget> CreateSettingsWidget();

	/** Handle merge button clicked */
	FReply OnMergeClicked();

	/** Handle refresh button clicked */
	FReply OnRefreshClicked();

	/** Check if merge can be executed */
	bool CanExecuteMerge() const;

	/** Get preview text */
	FText GetPreviewText() const;

	/** Settings change handlers */
	void OnReplaceSourceActorsChanged(ECheckBoxState NewState);
	void OnDeleteSourceActorsChanged(ECheckBoxState NewState);
	void OnPreserveMaterialOverridesChanged(ECheckBoxState NewState);
	void OnPreserveCustomDataChanged(ECheckBoxState NewState);

	/** Checkbox state getters */
	ECheckBoxState GetReplaceSourceActorsState() const;
	ECheckBoxState GetDeleteSourceActorsState() const;
	ECheckBoxState GetPreserveMaterialOverridesState() const;
	ECheckBoxState GetPreserveCustomDataState() const;

private:
	/** ISM Actors List */
	TSharedPtr<SListView<TSharedPtr<FISMActorData>>> ISMActorListView;
	TArray<TSharedPtr<FISMActorData>> ISMActorList;

	/** Static Mesh Actors List */
	TSharedPtr<SListView<TSharedPtr<FStaticMeshActorData>>> StaticMeshActorListView;
	TArray<TSharedPtr<FStaticMeshActorData>> StaticMeshActorList;

	/** Preview text block */
	TSharedPtr<STextBlock> PreviewTextBlock;

	/** Merge button */
	TSharedPtr<SButton> MergeButton;

	/** Tool instance */
	UISMMergeTool* MergeTool = nullptr;

	/** Current settings */
	FISMMergeSettings CurrentSettings;

	/** Last refresh time to avoid too frequent updates */
	double LastRefreshTime = 0.0;
};