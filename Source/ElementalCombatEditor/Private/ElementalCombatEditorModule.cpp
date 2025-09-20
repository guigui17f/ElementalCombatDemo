#include "ElementalCombatEditorModule.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "ISMMerge/SISMMergeWidget.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FElementalCombatEditorModule"

static const FName ISMMergeTabName("ISMMergeTool");

void FElementalCombatEditorModule::StartupModule()
{
	// Register tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ISMMergeTabName,
		FOnSpawnTab::CreateRaw(this, &FElementalCombatEditorModule::OnSpawnISMMergeTab))
		.SetDisplayName(LOCTEXT("ISMMergeTabTitle", "ISM合并工具"))
		.SetTooltipText(LOCTEXT("ISMMergeTooltipText", "打开ISM合并工具"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	// Register menus
	RegisterMenus();
}

void FElementalCombatEditorModule::ShutdownModule()
{
	// Unregister menus
	UnregisterMenus();

	// Unregister tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ISMMergeTabName);
}

TSharedRef<SDockTab> FElementalCombatEditorModule::OnSpawnISMMergeTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SISMMergeWidget)
		];
}

void FElementalCombatEditorModule::RegisterMenus()
{
	// Use startup callback to ensure menus are registered at the right time
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FElementalCombatEditorModule::RegisterMenusInternal));
}

void FElementalCombatEditorModule::RegisterMenusInternal()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add to Tools menu
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	if (ToolsMenu)
	{
		FToolMenuSection& ToolsSection = ToolsMenu->FindOrAddSection("ElementalCombat");
		ToolsSection.Label = LOCTEXT("ElementalCombatSection", "ElementalCombat");

		ToolsSection.AddMenuEntry("OpenISMMergeTool",
			LOCTEXT("OpenISMMergeTool", "ISM合并工具"),
			LOCTEXT("OpenISMMergeToolTooltip", "打开ISM合并工具，将静态网格体Actors合并到实例化静态网格体组件中"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
			FUIAction(
				FExecuteAction::CreateRaw(this, &FElementalCombatEditorModule::OpenISMMergeWindow)
			)
		);
	}
}

void FElementalCombatEditorModule::UnregisterMenus()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FElementalCombatEditorModule::OpenISMMergeWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ISMMergeTabName);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FElementalCombatEditorModule, ElementalCombatEditor)