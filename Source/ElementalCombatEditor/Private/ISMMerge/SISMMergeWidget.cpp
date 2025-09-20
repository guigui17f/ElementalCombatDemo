#include "ISMMerge/SISMMergeWidget.h"
#include "ISMMerge/ISMMergeTool.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Views/SListView.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/StaticMesh.h"

#define LOCTEXT_NAMESPACE "SISMMergeWidget"

//////////////////////////////////////////////////////////////////////////
// SISMActorItem

void SISMActorItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	ActorData = InArgs._ActorData;
	OnSelectionChanged = InArgs._OnSelectionChanged;

	STableRow<TSharedPtr<FISMActorData>>::Construct(
		STableRow<TSharedPtr<FISMActorData>>::FArguments()
		.Padding(2.0f),
		InOwnerTableView
	);

	SetContent(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SISMActorItem::IsChecked)
			.OnCheckStateChanged(this, &SISMActorItem::OnCheckStateChanged)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(this, &SISMActorItem::GetActorDisplayText)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(this, &SISMActorItem::GetISMComponentCountText)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
	);
}

ECheckBoxState SISMActorItem::IsChecked() const
{
	return ActorData.IsValid() ? (ActorData->bIsSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked) : ECheckBoxState::Unchecked;
}

void SISMActorItem::OnCheckStateChanged(ECheckBoxState NewState)
{
	if (ActorData.IsValid())
	{
		ActorData->bIsSelected = (NewState == ECheckBoxState::Checked);
		OnSelectionChanged.ExecuteIfBound(ActorData->bIsSelected);
	}
}

FText SISMActorItem::GetActorDisplayText() const
{
	if (ActorData.IsValid() && ActorData->Actor.IsValid())
	{
		return FText::FromString(ActorData->Actor->GetName());
	}
	return LOCTEXT("InvalidActor", "无效Actor");
}

FText SISMActorItem::GetISMComponentCountText() const
{
	if (ActorData.IsValid())
	{
		int32 ValidComponents = 0;
		for (const TWeakObjectPtr<UInstancedStaticMeshComponent>& Component : ActorData->ISMComponents)
		{
			if (Component.IsValid())
			{
				ValidComponents++;
			}
		}
		return FText::Format(LOCTEXT("ISMComponentCount", "({0} 个ISM组件)"), FText::AsNumber(ValidComponents));
	}
	return FText::GetEmpty();
}

//////////////////////////////////////////////////////////////////////////
// SStaticMeshActorItem

void SStaticMeshActorItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	ActorData = InArgs._ActorData;
	OnSelectionChanged = InArgs._OnSelectionChanged;

	STableRow<TSharedPtr<FStaticMeshActorData>>::Construct(
		STableRow<TSharedPtr<FStaticMeshActorData>>::FArguments()
		.Padding(2.0f),
		InOwnerTableView
	);

	SetContent(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SStaticMeshActorItem::IsChecked)
			.OnCheckStateChanged(this, &SStaticMeshActorItem::OnCheckStateChanged)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.4f)
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(this, &SStaticMeshActorItem::GetActorDisplayText)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.3f)
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(this, &SStaticMeshActorItem::GetStaticMeshText)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.3f)
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(this, &SStaticMeshActorItem::GetCompatibilityText)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(this, &SStaticMeshActorItem::GetCompatibilityTextColor)
		]
	);
}

ECheckBoxState SStaticMeshActorItem::IsChecked() const
{
	return ActorData.IsValid() ? (ActorData->bIsSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked) : ECheckBoxState::Unchecked;
}

void SStaticMeshActorItem::OnCheckStateChanged(ECheckBoxState NewState)
{
	if (ActorData.IsValid())
	{
		ActorData->bIsSelected = (NewState == ECheckBoxState::Checked);
		OnSelectionChanged.ExecuteIfBound(ActorData->bIsSelected);
	}
}

FText SStaticMeshActorItem::GetActorDisplayText() const
{
	if (ActorData.IsValid() && ActorData->Actor.IsValid())
	{
		return FText::FromString(ActorData->Actor->GetName());
	}
	return LOCTEXT("InvalidActor", "无效Actor");
}

FText SStaticMeshActorItem::GetStaticMeshText() const
{
	if (ActorData.IsValid() && ActorData->StaticMeshComponent.IsValid())
	{
		if (UStaticMesh* StaticMesh = ActorData->StaticMeshComponent->GetStaticMesh())
		{
			return FText::FromString(StaticMesh->GetName());
		}
	}
	return LOCTEXT("NoMesh", "无网格体");
}

FText SStaticMeshActorItem::GetCompatibilityText() const
{
	if (ActorData.IsValid())
	{
		if (ActorData->CompatibilityResult.bIsCompatible)
		{
			return LOCTEXT("Compatible", "兼容");
		}
		else if (!ActorData->CompatibilityResult.IncompatibilityReason.IsEmpty())
		{
			return FText::FromString(ActorData->CompatibilityResult.IncompatibilityReason);
		}
		else
		{
			return LOCTEXT("NotChecked", "未检查");
		}
	}
	return FText::GetEmpty();
}

FSlateColor SStaticMeshActorItem::GetCompatibilityTextColor() const
{
	if (ActorData.IsValid())
	{
		if (ActorData->CompatibilityResult.bIsCompatible)
		{
			return FSlateColor(FLinearColor::Green);
		}
		else if (!ActorData->CompatibilityResult.IncompatibilityReason.IsEmpty())
		{
			return FSlateColor(FLinearColor::Red);
		}
	}
	return FSlateColor::UseSubduedForeground();
}

//////////////////////////////////////////////////////////////////////////
// SISMMergeWidget

void SISMMergeWidget::Construct(const FArguments& InArgs)
{
	MergeTool = UISMMergeTool::Get();
	CurrentSettings = MergeTool->GetSettings();
	LastRefreshTime = 0.0;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(4.0f)
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ISMMergeToolTitle", "ISM合并工具"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.Justification(ETextJustify::Center)
			]

			// Main content
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SSplitter)
				.Orientation(Orient_Vertical)

				// Actor Lists
				+ SSplitter::Slot()
				.Value(0.6f)
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					// ISM Actors List
					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(2.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ISMActorsTitle", "ISM Actors"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(2.0f)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
							[
								SAssignNew(ISMActorListView, SListView<TSharedPtr<FISMActorData>>)
								.ListItemsSource(&ISMActorList)
								.OnGenerateRow(this, &SISMMergeWidget::OnGenerateISMActorRow)
								.SelectionMode(ESelectionMode::None)
							]
						]
					]

					// Static Mesh Actors List
					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(2.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("StaticMeshActorsTitle", "静态网格体Actors"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(2.0f)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
							[
								SAssignNew(StaticMeshActorListView, SListView<TSharedPtr<FStaticMeshActorData>>)
								.ListItemsSource(&StaticMeshActorList)
								.OnGenerateRow(this, &SISMMergeWidget::OnGenerateStaticMeshActorRow)
								.SelectionMode(ESelectionMode::None)
							]
						]
					]
				]

				// Settings and Controls
				+ SSplitter::Slot()
				.Value(0.4f)
				[
					SNew(SVerticalBox)

					// Settings
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4.0f)
					[
						CreateSettingsWidget()
					]

					// Preview
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
						.Padding(8.0f)
						[
							SAssignNew(PreviewTextBlock, STextBlock)
							.Text(this, &SISMMergeWidget::GetPreviewText)
							.AutoWrapText(true)
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						]
					]

					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(2.0f)
						[
							SNew(SButton)
							.Text(LOCTEXT("RefreshButton", "刷新"))
							.OnClicked(this, &SISMMergeWidget::OnRefreshClicked)
							.HAlign(HAlign_Center)
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(2.0f)
						[
							SAssignNew(MergeButton, SButton)
							.Text(LOCTEXT("MergeButton", "合并"))
							.OnClicked(this, &SISMMergeWidget::OnMergeClicked)
							.IsEnabled(this, &SISMMergeWidget::CanExecuteMerge)
							.HAlign(HAlign_Center)
						]
					]
				]
			]
		]
	];

	// Initial refresh
	RefreshActorLists();
}

void SISMMergeWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Auto-refresh every 2 seconds
	if (InCurrentTime - LastRefreshTime > 2.0)
	{
		RefreshActorLists();
		LastRefreshTime = InCurrentTime;
	}
}

void SISMMergeWidget::RefreshActorLists()
{
	// Collect ISM actors
	TArray<FISMActorData> NewISMActors = MergeTool->CollectISMActorsFromSelection();
	ISMActorList.Reset();
	for (const FISMActorData& ActorData : NewISMActors)
	{
		ISMActorList.Add(MakeShared<FISMActorData>(ActorData));
	}

	// Collect static mesh actors
	TArray<FStaticMeshActorData> NewStaticMeshActors = MergeTool->CollectStaticMeshActorsFromSelection();
	StaticMeshActorList.Reset();
	for (const FStaticMeshActorData& ActorData : NewStaticMeshActors)
	{
		StaticMeshActorList.Add(MakeShared<FStaticMeshActorData>(ActorData));
	}

	// Update compatibility
	UpdateCompatibilityResults();

	// Refresh lists
	if (ISMActorListView.IsValid())
	{
		ISMActorListView->RequestListRefresh();
	}
	if (StaticMeshActorListView.IsValid())
	{
		StaticMeshActorListView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> SISMMergeWidget::OnGenerateISMActorRow(TSharedPtr<FISMActorData> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SISMActorItem, OwnerTable)
		.ActorData(Item)
		.OnSelectionChanged(this, &SISMMergeWidget::OnISMActorSelectionChanged);
}

TSharedRef<ITableRow> SISMMergeWidget::OnGenerateStaticMeshActorRow(TSharedPtr<FStaticMeshActorData> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SStaticMeshActorItem, OwnerTable)
		.ActorData(Item)
		.OnSelectionChanged(this, &SISMMergeWidget::OnStaticMeshActorSelectionChanged);
}

void SISMMergeWidget::OnISMActorSelectionChanged(bool bIsSelected)
{
	UpdateCompatibilityResults();
}

void SISMMergeWidget::OnStaticMeshActorSelectionChanged(bool bIsSelected)
{
	// No additional action needed for static mesh selection change
}

void SISMMergeWidget::UpdateCompatibilityResults()
{
	// Convert shared pointers to raw data for the tool
	TArray<FISMActorData> ISMActorData;
	for (const TSharedPtr<FISMActorData>& Item : ISMActorList)
	{
		if (Item.IsValid())
		{
			ISMActorData.Add(*Item);
		}
	}

	// Update compatibility for each static mesh actor
	for (TSharedPtr<FStaticMeshActorData>& Item : StaticMeshActorList)
	{
		if (Item.IsValid() && Item->StaticMeshComponent.IsValid())
		{
			UInstancedStaticMeshComponent* BestTarget = MergeTool->FindBestISMTarget(Item->StaticMeshComponent.Get(), ISMActorData);
			if (BestTarget)
			{
				Item->CompatibilityResult = FISMCompatibilityResult(true);
				Item->CompatibilityResult.TargetComponent = BestTarget;
			}
			else
			{
				Item->CompatibilityResult = FISMCompatibilityResult(false, TEXT("No compatible ISM found"));
			}
		}
	}
}

TSharedRef<SWidget> SISMMergeWidget::CreateSettingsWidget()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MergeOptionsTitle", "合并选项"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SISMMergeWidget::GetReplaceSourceActorsState)
			.OnCheckStateChanged(this, &SISMMergeWidget::OnReplaceSourceActorsChanged)
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ReplaceSourceActors", "替换源Actors"))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f, 4.0f, 4.0f, 4.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SISMMergeWidget::GetDeleteSourceActorsState)
			.OnCheckStateChanged(this, &SISMMergeWidget::OnDeleteSourceActorsChanged)
			.IsEnabled_Lambda([this]() { return CurrentSettings.bReplaceSourceActors; })
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DeleteSourceActors", "删除源Actors（否则仅隐藏）"))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SISMMergeWidget::GetPreserveMaterialOverridesState)
			.OnCheckStateChanged(this, &SISMMergeWidget::OnPreserveMaterialOverridesChanged)
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PreserveMaterialOverrides", "保留材质覆盖"))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SCheckBox)
			.IsChecked(this, &SISMMergeWidget::GetPreserveCustomDataState)
			.OnCheckStateChanged(this, &SISMMergeWidget::OnPreserveCustomDataChanged)
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PreserveCustomData", "保留自定义数据"))
			]
		];
}

FReply SISMMergeWidget::OnMergeClicked()
{
	if (!CanExecuteMerge())
	{
		return FReply::Handled();
	}

	// Convert shared pointers to raw data
	TArray<FISMActorData> ISMActorData;
	for (const TSharedPtr<FISMActorData>& Item : ISMActorList)
	{
		if (Item.IsValid())
		{
			ISMActorData.Add(*Item);
		}
	}

	TArray<FStaticMeshActorData> StaticMeshActorData;
	for (const TSharedPtr<FStaticMeshActorData>& Item : StaticMeshActorList)
	{
		if (Item.IsValid())
		{
			StaticMeshActorData.Add(*Item);
		}
	}

	// Execute merge
	MergeTool->SetSettings(CurrentSettings);
	bool bSuccess = MergeTool->ExecuteMerge(ISMActorData, StaticMeshActorData, CurrentSettings);

	if (bSuccess)
	{
		// Refresh the lists after successful merge
		RefreshActorLists();
	}

	return FReply::Handled();
}

FReply SISMMergeWidget::OnRefreshClicked()
{
	RefreshActorLists();
	return FReply::Handled();
}

bool SISMMergeWidget::CanExecuteMerge() const
{
	// Check if we have at least one selected ISM actor and one selected static mesh actor
	bool bHasSelectedISM = false;
	bool bHasSelectedStaticMesh = false;

	for (const TSharedPtr<FISMActorData>& Item : ISMActorList)
	{
		if (Item.IsValid() && Item->bIsSelected)
		{
			bHasSelectedISM = true;
			break;
		}
	}

	for (const TSharedPtr<FStaticMeshActorData>& Item : StaticMeshActorList)
	{
		if (Item.IsValid() && Item->bIsSelected && Item->CompatibilityResult.bIsCompatible)
		{
			bHasSelectedStaticMesh = true;
			break;
		}
	}

	return bHasSelectedISM && bHasSelectedStaticMesh;
}

FText SISMMergeWidget::GetPreviewText() const
{
	int32 SelectedISMCount = 0;
	int32 SelectedStaticMeshCount = 0;
	int32 CompatibleCount = 0;

	for (const TSharedPtr<FISMActorData>& Item : ISMActorList)
	{
		if (Item.IsValid() && Item->bIsSelected)
		{
			SelectedISMCount++;
		}
	}

	for (const TSharedPtr<FStaticMeshActorData>& Item : StaticMeshActorList)
	{
		if (Item.IsValid() && Item->bIsSelected)
		{
			SelectedStaticMeshCount++;
			if (Item->CompatibilityResult.bIsCompatible)
			{
				CompatibleCount++;
			}
		}
	}

	if (SelectedISMCount == 0 && SelectedStaticMeshCount == 0)
	{
		return LOCTEXT("NoSelection", "请选择ISM Actors和静态网格体Actors以开始合并。");
	}
	else if (SelectedISMCount == 0)
	{
		return LOCTEXT("NoISMSelected", "请至少选择一个ISM Actor。");
	}
	else if (SelectedStaticMeshCount == 0)
	{
		return LOCTEXT("NoStaticMeshSelected", "请至少选择一个静态网格体Actor。");
	}
	else if (CompatibleCount == 0)
	{
		return LOCTEXT("NoCompatibleMeshes", "未找到兼容的静态网格体Actors。");
	}
	else
	{
		return FText::Format(
			LOCTEXT("PreviewFormat", "准备将 {0} 个兼容的静态网格体Actors合并到 {1} 个ISM Actors中。"),
			FText::AsNumber(CompatibleCount),
			FText::AsNumber(SelectedISMCount)
		);
	}
}

void SISMMergeWidget::OnReplaceSourceActorsChanged(ECheckBoxState NewState)
{
	CurrentSettings.bReplaceSourceActors = (NewState == ECheckBoxState::Checked);
}

void SISMMergeWidget::OnDeleteSourceActorsChanged(ECheckBoxState NewState)
{
	CurrentSettings.bDeleteSourceActors = (NewState == ECheckBoxState::Checked);
}

void SISMMergeWidget::OnPreserveMaterialOverridesChanged(ECheckBoxState NewState)
{
	CurrentSettings.bPreserveMaterialOverrides = (NewState == ECheckBoxState::Checked);
}

void SISMMergeWidget::OnPreserveCustomDataChanged(ECheckBoxState NewState)
{
	CurrentSettings.bPreserveCustomData = (NewState == ECheckBoxState::Checked);
}

ECheckBoxState SISMMergeWidget::GetReplaceSourceActorsState() const
{
	return CurrentSettings.bReplaceSourceActors ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SISMMergeWidget::GetDeleteSourceActorsState() const
{
	return CurrentSettings.bDeleteSourceActors ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SISMMergeWidget::GetPreserveMaterialOverridesState() const
{
	return CurrentSettings.bPreserveMaterialOverrides ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SISMMergeWidget::GetPreserveCustomDataState() const
{
	return CurrentSettings.bPreserveCustomData ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

#undef LOCTEXT_NAMESPACE