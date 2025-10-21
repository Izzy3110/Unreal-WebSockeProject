/**
 * InstaMATPCH.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATPCH.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#ifndef InstaMAT_InstaMATPCH_h
#define InstaMAT_InstaMATPCH_h

#include "UnrealEd.h"
#include "CoreTypes.h"

#include "InstaMAT/InstaMATAPI.h"

class InstaMATShared
{
public:
	static FString Version;
	static FString LicenseInformation;
	
	static void OpenAuthorizationWindowModal();
	static void OpenDeauthorizationWindowModal();
	/**
	 * Shows an dialog with a path selection.
	 *
	 * @return The selected Path.
	 */
	static FString OpenInstaMATStudioSelectionWindowModal();

	/**
	 * Shows an error message dialog.
	 *
	 * @param Title The message title.
	 * @param MessageTitle The message title.
	 * @param MessageContent The message content.
	 */
	static void OpenInstaMATErrorMessageDialog(const FText& Title, const FText& MessageTitle, const FText& MessageContent);

	/**
	 * Method for opening the floating license unavailable window.
	 *
	 * @param ForceLicenseRefreshCallback callback function to be called to refresh the license.
	 */
	static void OpenFloatingLicenseUnavailableWindowModal(InstaMAT::IInstaMAT::pfnForceLicenseRefreshCallback ForceLicenseRefreshCallback);
};

DEFINE_LOG_CATEGORY_STATIC(LogInstaMAT, Log, All);

#endif
