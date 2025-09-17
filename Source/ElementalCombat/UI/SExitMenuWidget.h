// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

DECLARE_DELEGATE_OneParam(FOnExitMenuResult, bool /* bConfirmed */);

/**
 * Slate widget for exit confirmation menu
 * Shows a modal dialog with Confirm/Cancel options when ESC is pressed
 */
class ELEMENTALCOMBAT_API SExitMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SExitMenuWidget) {}
		SLATE_EVENT(FOnExitMenuResult, OnResult)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** Get the desired size of this widget */
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	/** Make widget support keyboard focus */
	virtual bool SupportsKeyboardFocus() const override { return true; }

protected:
	/** Called when Confirm button is clicked */
	FReply OnConfirmClicked();

	/** Called when Cancel button is clicked */
	FReply OnCancelClicked();

	/** Called when ESC key is pressed */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	/** Delegate to call when user makes a choice */
	FOnExitMenuResult OnResultDelegate;

	/** Size of the dialog */
	static const FVector2D DialogSize;
};