/**
 * InstaMATImporterModule.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATImporterModule.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"

/**
 * The FInstaMATImporterModule initializes the Importer factory for InstaMAT.
 */
class INSTAMATIMPORTER_API FInstaMATImporterModule : public IModuleInterface
{
public:

	/**
	 * Function called upon engine initialization.
	 */
	virtual void StartupModule() override;

	/**
	 * Function called upon engine shutdown.
	 */
	virtual void ShutdownModule() override;
};
