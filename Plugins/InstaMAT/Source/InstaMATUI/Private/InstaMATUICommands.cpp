/**
 * InstaMATUICommands.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATUICommands.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATUICommands.h"
#include "InstaMATUIPCH.h"
#include "Slate/InstaMATPluginStyle.h"

#define LOCTEXT_NAMESPACE "FInstaMATUIModule"

FInstaMATUICommands::FInstaMATUICommands() : TCommands<FInstaMATUICommands>(TEXT("InstaMATUI"), NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATUI", "InstaMAT Plugin"), NAME_None, FInstaMATPluginStyle::GetStyleSetName())
{
}

void FInstaMATUICommands::RegisterCommands()
{
	UI_COMMAND(OpenInstaMATSettingsWindow, "InstaMAT", "Opens up a dockable window for the InstaMAT plugin settings.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenInstaMATLibraryWindow, "InstaMAT", "Opens up a dockable window for the InstaMAT plugin library.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
