/**
 * InstaMATSettings.h (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSettings.h
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#pragma once

#include "CoreMinimal.h"
#include "InstaMAT/InstaMATEnum.h"
#include "InstaMATSettings.generated.h"

/**
 * Struct for the InstaMAT user directories.
 */
USTRUCT()
struct FInstaMATUserDirectory
{
	GENERATED_BODY()

	/** The user path. */
	UPROPERTY(Config, EditAnywhere, meta = (DisplayName = "User Path"))
	FDirectoryPath UserPath;

	/** Whether this instance is the default path. */
	UPROPERTY(Config, EditAnywhere, meta = (DisplayName = "Default Path"))
	bool bIsDefault = false;
};

/**
 * Base Class for the Settings tab.
 */
UCLASS(hideCategories = (Object, "Utilities"), config = EditorPerProjectUserSettings, defaultconfig)
class INSTAMAT_API UInstaMATSettings : public UObject
{
	GENERATED_BODY()

	/// VARIABLES ///

public:

	/** License Information */
	UPROPERTY(EditAnywhere, Category = "LicenseInfo")
	FText LicenseInformation;

	/** Account Name */
	UPROPERTY(Config, EditAnywhere, meta = (DisplayName = "Account"), Category = "Settings")
	FText AccountName;

	/** Serial/Password */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Serial/Password"), Category = "Settings")
	FText SerialPassword;

	/** The width resolution. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "Resolution Width"), Category = "Texture Settings")
	EInstaMATTextureSize ResolutionWidth = EInstaMATTextureSize::InstaMAT_2K;

	/** The height resolution. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "Resolution Height"), Category = "Texture Settings")
	EInstaMATTextureSize ResolutionHeight = EInstaMATTextureSize::InstaMAT_2K;

	/** The Execution Format. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "Execution Format"), Category = "Texture Settings")
	EInstaMATExecutionFormat ExecutionFormat = EInstaMATExecutionFormat::InstaMAT_Normalized8;

	/** The width resolution. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "Preview Mode Resolution Width"), Category = "Preview Texture Settings")
	EInstaMATTextureSize PreviewResolutionWidth = EInstaMATTextureSize::InstaMAT_512;

	/** The height resolution. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "Preview Mode Resolution Height"), Category = "Preview Texture Settings")
	EInstaMATTextureSize PreviewResolutionHeight = EInstaMATTextureSize::InstaMAT_512;

	/** Locks the Width and Height to be equal. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "Lock Resolution"), Category = "Texture Settings")
	bool bLockTextureResolution = true;

	/** Whether the shading model of the created material should switch to subsurface if a subsurface texture is available in the graph instance. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "Enable Subsurface Shading Model"), Category = "Material Settings")
	bool bIsShadingModelSubsurfaceEnabled = false;

	/** The InstaMAT environment folder. */
	UPROPERTY(Config, EditAnyWhere, meta = (ContentDir, DisplayName = "InstaMAT Studio installation folder"), Category = "Path Settings")
	FString EnvironmentFolder;

	/** The InstaMAT user folders. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "User Paths"), Category = "Path Settings")
	TArray<FInstaMATUserDirectory> UserFolders;

	/** The Delay between the last InstaMAT Graph update and UE resource update. */
	UPROPERTY(Config, EditAnyWhere, meta = (ContentDir, DisplayName = "Update Delay", ClampMin = "0.5", ClampMax = "10.0", UIMin = "0.5", UIMax = "10.0"), Category = "UI Settings")
	float Delay = 2.0f;

	/** Whether not supported input types are visible in Graph Instances. */
	UPROPERTY(Config, EditAnyWhere, meta = (DisplayName = "Is Not Supported Types Visible"), Category = "UI Settings")
	bool bIsUnsupportedTypesVisible = true;

	/** Memory Budget. */
	UPROPERTY(Config, EditAnyWhere, meta = (ContentDir, DisplayName = "Video Memory Budget", ClampMin = "-1", UIMin = "-1"), Category = "VRAM Settings")
	int64 VRAMBudget = -1;

	/** Enables custom memory budget. */
	UPROPERTY(Config, EditAnyWhere, meta = (ContentDir, DisplayName = "Enable Custom Budget", ClampMin = "-1", UIMin = "-1"), Category = "VRAM Settings")
	bool bEnableCustomMemorySetting = true;

	/** Whether the execution is processed in an asynchronous manner.*/
	UPROPERTY(Config, EditAnyWhere, meta = (ContentDir, DisplayName = "Execute Asynchronously"), Category = "Execution Settings")
	bool bExecuteAsync = true;

	/** Whether the execution is triggered automatically. */
	UPROPERTY(Config, EditAnywhere, meta = (DisplayName = "Default Update Type"), Category = "Execution Settings")
	EInstaMATUpdateType DefaultUpdateType = EInstaMATUpdateType::InstaMAT_Automatic;

	/** Authorized? */
	bool bIsAuthorized = false;

	/** Whether the user paths changed. Changing user paths requires updating InstaMAT library window. */
	bool bIsUserPathsChanged = false;

	/// FUNCTIONS ///	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;

	/**
	 * Whether custom VRAM settings are available.
	 *
	 * @return True if enough VRAM is available on the GPU.
	 */
	bool IsCustomVRAMSettingAvailable();

	/** Saves the settings of this instance to the default object. */
	void SaveToDefaultObject();

	/** Ensures a valid Default User path is set. */
	void EnsureDefaultUserPathIsSet();

	/** Determines whether InstaMAT is configured correctly. */
	static bool IsInstaMATConfigured();

	/** Constructor */
	UInstaMATSettings();

	class SInstaMATSettingsWindow* GetInstaMATSettingsWindow() const { return InstaMATSettingsWindow; }
	void SetInstaMATSettingsWindow(class SInstaMATSettingsWindow* NewWindow) { InstaMATSettingsWindow = NewWindow; }
	class IInstaMAT* GetInstaMATInterface() const { return InstaMAT; }

	/************************************************************************/
	/* Utilities                                                            */
	/************************************************************************/

	/** Authorizes the workstation. */
	UFUNCTION(Exec, meta = (DisplayName = "Authorize Workstation"), Category = "Authorize")
		void AuthorizeWorkstation();

	/** Deauthorizes the workstation. */
	UFUNCTION(Exec, meta = (DisplayName = "Deauthorize Workstation"), Category = "Deauthorize")
		void DeauthorizeWorkstation();

	/** Try to authorize refresh a floating license again for this workstation. */
	UFUNCTION(Exec, meta = (DisplayName = "Try Again"), Category = "FloatingLicense")
		void TryFloatingLicenseAgain_Exec();

	/** Refreshes the External Assets folders from registered user paths. */
	UFUNCTION(Exec, meta = (DisplayName = "Reload External Assets"), Category = "Path Settings")
	void ReloadExternalAssets(bool bDisplaySuccessMessage = true);

	/** Try to authorize refresh a floating license again for this workstation. */
	static bool TryFloatingLicenseAgain();

	/** Restarts the library window. */
	static void RestartLibraryWindow();

	/** Restarts the InstaMAT Settings window. */
	static void RestartSettingsWindow();

	/** Restarts all InstaMAT windows. */
	static void RestartAllInstaMATWindows();

	/************************************************************************/
	/* Getters / Setters                                                    */
	/************************************************************************/

	/** Returns License Information. */
	FText GetLicenseInfo() const { return LicenseInformation; }

	/** Whether the execution is processed in an asynchronous manner.*/
	bool ExecuteAsync() const { return bExecuteAsync; }

	/** Delegate which is called after changes which would require a UI to be rebuild. */
	FSimpleDelegate UpdateDelegate;

	/** Gets the application directory. */
	static FString GetApplicationDirectory();

	/** Minimum memory budget. */
	static const int64 kMinimumMemoryBudget;

protected:
	/** The InstaMAT Settings window. */
	class SInstaMATSettingsWindow* InstaMATSettingsWindow;
	/** InstaMAT SDK object. */
	class IInstaMAT* InstaMAT;

private:
	/** Notifies the user if editor restart is necessary. */
	void NotifyRestartIfNecessary();

	TWeakPtr<SNotificationItem> RestartNotificationItem;	/** A pointer to the restart notification item. */

	uint32 EmptyPathsCount;									/** The empty paths count. */
};