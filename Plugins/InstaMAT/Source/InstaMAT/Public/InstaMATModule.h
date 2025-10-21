/**
 * InstaMATModule.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATModule.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */
#ifndef InstaMAT_InstaMATModule_h
#define InstaMAT_InstaMATModule_h

#include "MeshUtilities.h"
#include "InstaMAT/InstaMATAPI.h"
#include "InstaMAT/InstaMAT.h"
#include "Editor/UnrealEd/Public/EditorDirectories.h"

/**
 * The FInstaMATModule initializes the InstaMAT SDK. 
 */
class INSTAMAT_API FInstaMATModule : public IModuleInterface, public IModularFeature
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

	/**
	 * Returns the name of this module.
	 * 
	 * @return The name of this module.
	 */
	FString GetName()
	{
		return TEXT("InstaMAT");
	}

	/**
	 * Returns the InstaMAT Interface.
	 *
	 * @return The Interface.
	 */
	virtual IInstaMAT* GetInstaMATInterface();

	/**
	 * Returns the InstaMAT API.
	 *
	 * @return The API.
	 */
	InstaMAT::IInstaMAT* GetInstaMATAPI() const { check(InstaMATAPI != nullptr); return InstaMATAPI; }

	/**
	 * Determines whether InstaMAT is initialized.
	 * 
	 * @return True upon success. 
	 */
	bool IsInitialized();

	/**
	 * Returns the default path to display at the content browser window for the content browser type, with a fallback path
	 * in case no saved path is found.
	 *
	 * @param ContentBrowserType	The type of the ContentBrowser being displayed.
	 * @param FallbackPath			Optional. The path to use in case no saved path is found.
	 */
	FString GetDefaultPathForContentBrowser(ELastDirectory::Type ContentBrowserType, const FString& FallbackPath = FString());

	/**
	 * Called by the SDK license verification thread when the license becomes unavailable.
	 *
	 * @param ForceLicenseRefreshCallback A delegate to be called to force refresh the license when the floating license becomes unavailable.
	 */
	static void LicenseUnavailableCallback(InstaMAT::IInstaMAT::pfnForceLicenseRefreshCallback ForceLicenseRefreshCallback);

	/**
	* Checks if the InstaMAT executable is found in the specified \p Path.
	*
	* @param Path The search path.
	* @return True upon success.
	*/
	static bool IsInstaMATExecutableInPath(const FString& Path);

	static InstaMAT::IInstaMAT::pfnForceLicenseRefreshCallback ForceLicenseRefreshDelegate;	/**< A delegate to be called to force refresh the license when the floating license becomes unavailable. */
	static bool bIsInstaMATFloatingLicenseAvailable;										/**< Whether a floating license for the current authorized user is available. */

private:

	/**
	 * Installs hooks into the Engine.
	 */
	void InstallHooks();

	/**
	 * Installs hooks into the Engine after initialization.
	 */
	void InstallHooksLate();

	FDelegateHandle LateHooksDelegateHandle;	/**< Delegate for the late hook initialization. */
	InstaMAT::IInstaMAT* InstaMATAPI;			/**< The InstaMAT SDK object. */
};

#endif
