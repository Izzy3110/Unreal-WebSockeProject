/**
 * InstaMATUICommands.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATUICommands.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#ifndef InstaMAT_FInstaMATUICommands_h
#define InstaMAT_FInstaMATUICommands_h

#include "Framework/Commands/Commands.h"

/**
 * The FInstaMATUICommands keeps track of registered UI commands.
 */
class FInstaMATUICommands : public TCommands<FInstaMATUICommands>
{
public:
	FInstaMATUICommands();

	/**
	 * Registers the commands.
	 */
	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> OpenInstaMATSettingsWindow;	/**< Command used to open InstaMAT settings window. */
	TSharedPtr<FUICommandInfo> OpenInstaMATLibraryWindow;	/**< Command used to open InstaMAT library window. */
};

#endif
