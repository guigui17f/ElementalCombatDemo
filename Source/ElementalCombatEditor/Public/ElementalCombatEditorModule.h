#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FElementalCombatEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register menu extensions */
	void RegisterMenus();

	/** Internal menu registration function */
	void RegisterMenusInternal();

	/** Unregister menu extensions */
	void UnregisterMenus();

	/** Open ISM Merge Tool window */
	void OpenISMMergeWindow();

	/** Spawn ISM Merge Tool tab */
	TSharedRef<class SDockTab> OnSpawnISMMergeTab(const class FSpawnTabArgs& Args);

private:
	/** Tab spawner for ISM Merge Tool */
	TSharedPtr<class FTabManager::FSearchPreference> ISMMergeTabSearchPreference;
};