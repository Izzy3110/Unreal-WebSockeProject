/**
 * InstaMATSettings.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATSettings.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMAT/InstaMATSettings.h"
#include "InstaMAT/Public/Slate/InstaMATPluginStyle.h"
#include "InstaMATPCH.h"

#include "InstaMATModule.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include <Winreg.h>
#include <ShlObj.h>
#endif

#if PLATFORM_MAC
#include <pwd.h>
#include <unistd.h>
#endif

#define LOCTEXT_NAMESPACE "InstaMATUI"

const int64 UInstaMATSettings::kMinimumMemoryBudget = 1024l;

/**
 * The InstaMATDirectoryUtility has functions for retrieving user directories.
 */
namespace InstaMATDirectoryUtility
{
	/**
	 * Gets the user directory.
	 * 
	 * @return The user directory.
	 */
	static FString GetUserDirectory()
	{
		const FString StudioPath = "/InstaMAT";
#if PLATFORM_WINDOWS
		TCHAR DocumentsDirectory[MAX_PATH];

		if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, DocumentsDirectory) == S_OK)
		{
			FString UserPath = DocumentsDirectory + StudioPath;
			FPaths::NormalizeDirectoryName(UserPath);
			return UserPath;
		}
#endif
		
#if PLATFORM_MAC
		const char* HomeDirectory = getenv("HOME");
		
		if (HomeDirectory)
		{
			struct passwd* const UserWorkingDirectory = getpwuid(getuid());
			
			if (UserWorkingDirectory)
			{
				FString UserPath = FString(UserWorkingDirectory->pw_dir) + StudioPath;
				FPaths::NormalizeDirectoryName(UserPath);
				return UserPath;
			}
		}
#endif
		return FString();
	}

	/**
	 * Gets the application directory.
	 * 
	 * @return The application directory.
	 */
	static FString GetApplicationDirectory()
	{
#if PLATFORM_WINDOWS
		HKEY KeyHandle;
		const LSTATUS Result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InstaMAT Studio"), 0, KEY_READ, &KeyHandle);

		if (Result == ERROR_SUCCESS)
		{
			TCHAR Buffer[512];
			DWORD BufferSize = sizeof(Buffer);
			const LSTATUS KeyResult = RegQueryValueEx(KeyHandle, TEXT("InstallDir"), 0, nullptr, reinterpret_cast<LPBYTE>(Buffer), &BufferSize);

			if (KeyResult == ERROR_SUCCESS)
			{
				RegCloseKey(KeyHandle);
				return FString(Buffer);
			}
		}
#endif
		return FString();
	}
}

UInstaMATSettings::UInstaMATSettings() : UObject()
{
	if (!FModuleManager::Get().IsModuleLoaded(FName(TEXT("InstaMAT"))))
		return;

	FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	InstaMAT::IInstaMAT *const InstaMATAPI = InstaMATModule.GetInstaMATAPI();

	// fetch license information and version
	if (InstaMATAPI != nullptr)
	{
		LicenseInformation = FText::FromString(ANSI_TO_TCHAR(InstaMATAPI->GetAuthorizationInformation()));
		bIsAuthorized = InstaMATAPI->IsHostAuthorized();
	}
}

void UInstaMATSettings::AuthorizeWorkstation()
{
	if (AccountName.IsEmpty() == false && SerialPassword.IsEmpty() == false)
	{
		// callback to the InstaMATAPI
		FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
		if (InstaMATModule.GetInstaMATAPI() != nullptr)
		{
			if (InstaMATModule.GetInstaMATAPI()->AuthorizeMachine(TCHAR_TO_UTF8(*AccountName.ToString()), TCHAR_TO_UTF8(*SerialPassword.ToString())))
			{
				const FText MessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorization", "InstaMAT: Machine Authorization");
				const FText AuthorizedMessageContent = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationCompleted", "A valid InstaMAT license has been acquired for this machine.\n\nInstaMAT will automatically refresh the license if necessary.");
				FMessageDialog::Open(EAppMsgType::Ok, AuthorizedMessageContent, MessageTitle);
				
				LicenseInformation = FText::FromString(ANSI_TO_TCHAR(InstaMATModule.GetInstaMATAPI()->GetAuthorizationInformation()));
				bIsAuthorized = InstaMATModule.GetInstaMATAPI()->IsHostAuthorized();

				UpdateDelegate.ExecuteIfBound();
				RestartLibraryWindow();
			}
			else
			{
				const FText Title = NSLOCTEXT(LOCTEXT_NAMESPACE, "AuthorizeError_Title", "Authorization Error");
				const FString AuthorizationInformation = UTF8_TO_TCHAR(InstaMATModule.GetInstaMATAPI()->GetAuthorizationInformation());
				
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Failed to authorize machine:\n\n" + AuthorizationInformation), Title);
			}
		}
		else
		{
			const FText Title = NSLOCTEXT(LOCTEXT_NAMESPACE, "AuthorizeError_Title", "Couldn't retrieve InstaMAT Module.");
			FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("InstaMATUI", "AuthorizeError_Message", "Please make sure the InstaMAT Module loaded properly!"), Title);
		}

		return;
	}

	const FText Title = NSLOCTEXT(LOCTEXT_NAMESPACE, "AuthorizeMessage_Title", "No Username or Password!");
	FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("InstaMATUI", "AuthorizeMessage_Message", "Please enter Username and Password!"), Title);
}

void UInstaMATSettings::DeauthorizeWorkstation()
{
	if (!AccountName.IsEmpty() && !SerialPassword.IsEmpty())
	{
		// callback to the InstaMATAPI
		FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
		if (InstaMATModule.GetInstaMATAPI()->DeauthorizeMachine(TCHAR_TO_UTF8(*AccountName.ToString()), TCHAR_TO_UTF8(*SerialPassword.ToString())))
		{
			LicenseInformation = FText::FromString(ANSI_TO_TCHAR(InstaMATModule.GetInstaMATAPI()->GetAuthorizationInformation()));
			bIsAuthorized = InstaMATModule.GetInstaMATAPI()->IsHostAuthorized();

			UpdateDelegate.ExecuteIfBound();
			RestartLibraryWindow();
		}
		return;
	}

	const FText Title = NSLOCTEXT(LOCTEXT_NAMESPACE, "DeauthorizeMessage_Title", "No Username or Password!");
	FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT(LOCTEXT_NAMESPACE, "DeauthorizeMessage_Message", "Please enter Username and Password!"), Title);
}

void UInstaMATSettings::TryFloatingLicenseAgain_Exec()
{
	UInstaMATSettings::TryFloatingLicenseAgain();
}

void UInstaMATSettings::ReloadExternalAssets(bool bDisplaySuccessMessage /*= true*/)
{
	FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	IInstaMAT* const InstaMATInterface = InstaMATModule.GetInstaMATInterface();

	// Unregister all Assets Paths first.
	InstaMATInterface->UnregisterAllExternalAssetsFolder();

	// Loop through all User Paths and register them again.
	for (const FInstaMATUserDirectory& Directory : UserFolders)
	{
		if (Directory.UserPath.Path.IsEmpty())
			continue;

		InstaMATInterface->RegisterExternalAssetsFolder(FPaths::Combine(Directory.UserPath.Path, TEXT("Assets")));
	}

	if (bDisplaySuccessMessage)
	{
		const FText Title = NSLOCTEXT(LOCTEXT_NAMESPACE, "ReloadExternalAssets_Title", "External assets refresh");
		const FText Message = NSLOCTEXT(LOCTEXT_NAMESPACE, "ReloadExternalAssets_Message", "All registerd external assets paths were successfully refreshed.");

		FMessageDialog::Open(EAppMsgCategory::Success, EAppMsgType::Ok, Message, Title);
	}
}

bool UInstaMATSettings::TryFloatingLicenseAgain()
{
	FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>("InstaMAT");

	const char* ErrorMessage;
	if (InstaMATModule.ForceLicenseRefreshDelegate == nullptr || InstaMATModule.ForceLicenseRefreshDelegate(&ErrorMessage))
	{
		InstaMATModule.bIsInstaMATFloatingLicenseAvailable = true;
		InstaMATModule.ForceLicenseRefreshDelegate = nullptr;

		UInstaMATSettings::RestartAllInstaMATWindows();

		const FText SuccessTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATLicenseAquireSuccess", "Success");
		const FText SuccessfullLicenseMessage  = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATReAuthorizationErrorMessages", "InstaMAT: Aquired floating license successfully");
		FMessageDialog::Open(EAppMsgCategory::Success, EAppMsgType::Ok, SuccessfullLicenseMessage, SuccessTitle);

		return true;
	}
	
	const FString ErrorString(ErrorMessage);
	const FText ErrorMessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATReAuthorizationErrorMessages", "InstaMAT: Failed to verify the license");
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ErrorString), ErrorMessageTitle);
	return false;
}

void UInstaMATSettings::RestartLibraryWindow()
{
	static const FName InstaMATLibraryWindowTabName(TEXT("InstaMATLibraryWindow"));
	TWeakPtr<SDockTab> LibraryTab = FGlobalTabmanager::Get()->FindExistingLiveTab(InstaMATLibraryWindowTabName);
	if (!LibraryTab.IsValid())
		return;

	LibraryTab.Pin()->RequestCloseTab();
	LibraryTab = FGlobalTabmanager::Get()->TryInvokeTab(InstaMATLibraryWindowTabName);
	if (!LibraryTab.IsValid())
	{
		const FText Title = FText::FromString(TEXT("InstaMAT"));
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ReOpeningMATLibraryFailedMessage", "Failed to restart MAT library window. Please re-open MAT library window to see the changes."), Title);
	}
}

void UInstaMATSettings::RestartSettingsWindow()
{
	static const FName InstaMATSettingsWindowTabName(TEXT("InstaMATSettingsWindow"));
	TWeakPtr<SDockTab> SettingsTab = FGlobalTabmanager::Get()->FindExistingLiveTab(InstaMATSettingsWindowTabName);
	if (!SettingsTab.IsValid())
		return;

	SettingsTab.Pin()->RequestCloseTab();
	SettingsTab = FGlobalTabmanager::Get()->TryInvokeTab(InstaMATSettingsWindowTabName);
	if (!SettingsTab.IsValid())
	{
		const FText Title = FText::FromString(TEXT("InstaMAT"));
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ReOpeningMATSettingsFailedMessage", "Failed to restart MAT settings window. Please manually re-open MAT settings window to see the changes."), Title);
	}
}

void UInstaMATSettings::RestartAllInstaMATWindows()
{
	RestartLibraryWindow();
	RestartSettingsWindow();
}

void UInstaMATSettings::PreEditChange(FProperty* PropertyAboutToChange)
{
	if (PropertyAboutToChange != nullptr)
	{
		if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UInstaMATSettings, UserFolders) || PropertyAboutToChange->GetFName().Compare(TEXT("Path")) == 0)
		{
			EmptyPathsCount = 0u;
			for (int32 Index = 0; Index < UserFolders.Num(); Index++)
			{
				const FInstaMATUserDirectory& UserDirectory = UserFolders[Index];
				if (!UserDirectory.UserPath.Path.IsEmpty())
					continue;

				EmptyPathsCount++;
			}
		}
	}

	Super::PreEditChange(PropertyAboutToChange);
}

void UInstaMATSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	// handle user folder settings
	if (PropertyChangedEvent.GetPropertyName().Compare(TEXT("bIsDefault")) == 0)
	{
		const int32 ValueIndex = PropertyChangedEvent.GetArrayIndex(TEXT("UserFolders"));
		check(UserFolders.Num() > ValueIndex);

		if (ValueIndex != -1)
		{
			if (UserFolders[ValueIndex].bIsDefault)
			{
				// disable all others
				for (int32 Index = 0; Index < UserFolders.Num(); Index++)
				{
					if (ValueIndex == Index)
						continue;

					UserFolders[Index].bIsDefault = false;
				}
			}
			else
			{
				if (UserFolders.Num() > 0)
				{
					UserFolders[0].bIsDefault = true;
				}
			}
		}
	}
	else if (PropertyChangedEvent.GetPropertyName().Compare(TEXT("UserFolders")) == 0)
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
		{
			const int32 ValueIndex = PropertyChangedEvent.GetArrayIndex(TEXT("UserFolders"));
			check(UserFolders.Num() > ValueIndex);

			FString Path;
			if (ValueIndex != -1)
			{
				FInstaMATUserDirectory& Directory = UserFolders[ValueIndex];
				Path = Directory.UserPath.Path;

				if (!Directory.UserPath.Path.IsEmpty())
				{
					FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
					IInstaMAT* const InstaMATInterface = InstaMATModule.GetInstaMATInterface();
					InstaMATInterface->LoadEnvironmentPackageFromPath(TCHAR_TO_UTF8(*Directory.UserPath.Path), /*bIsSystemLibrary:*/ false);

					const FString AssetsPath = FPaths::Combine(Directory.UserPath.Path, TEXT("Assets"));
					InstaMATInterface->RegisterExternalAssetsFolder(AssetsPath);
				}
			}
		}
		else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Duplicate || 
			PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
		{
			EnsureDefaultUserPathIsSet();
		}
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
		{
			NotifyRestartIfNecessary();
		}
	}
	else if (PropertyChangedEvent.GetPropertyName().Compare(TEXT("Path")) == 0)
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
		{
			const FText MessageTitle = FText::FromString(TEXT("InstaMAT"));
			const int32 ValueIndex = PropertyChangedEvent.GetArrayIndex(TEXT("UserFolders"));
			FInstaMATUserDirectory EditedDirectory;
			if (ValueIndex != -1)
			{
				EditedDirectory = UserFolders[ValueIndex];
			}
			NotifyRestartIfNecessary();
			EnsureDefaultUserPathIsSet();

			FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
			IInstaMAT* const InstaMATInterface = InstaMATModule.GetInstaMATInterface();
			check(InstaMATInterface != nullptr);

			bIsUserPathsChanged = false;
			for (const FInstaMATUserDirectory& Directory : UserFolders)
			{
				if (Directory.UserPath.Path.IsEmpty())
					continue;

				const uint32 LoadedPackages = InstaMATInterface->LoadEnvironmentPackageFromPath(Directory.UserPath.Path, /*bIsSystemLibrary:*/ false);

				// Register the Assets path from the user path as external assets folder.
				InstaMATInterface->RegisterExternalAssetsFolder(FPaths::Combine(Directory.UserPath.Path, TEXT("Assets")));

				if (!EditedDirectory.UserPath.Path.IsEmpty() && EditedDirectory.UserPath.Path == Directory.UserPath.Path)
				{
					FText MessageText;
					switch (LoadedPackages)
					{
						case 0u:
							MessageText = LOCTEXT("PackageLoadedMessage", "Added new User Path successfully.");
							break;
						case 1u:
							MessageText = FText::Format(LOCTEXT("PackageLoadedMessage", "Added new User Path and loaded {0} package successfully."), FText::FromString(FString::FromInt(LoadedPackages)));
							break;
						default:
							MessageText = FText::Format(LOCTEXT("PackageLoadedMessage", "Added new User Path and loaded {0} packages successfully."), FText::FromString(FString::FromInt(LoadedPackages)));
							break;
					}
					FMessageDialog::Open(EAppMsgType::Ok, MessageText, MessageTitle);
				}
				UE_LOG(LogInstaMAT, Display, TEXT("InstaMAT: Loaded %u packages from='%s'"), LoadedPackages, *(Directory.UserPath.Path))

				bIsUserPathsChanged = bIsUserPathsChanged || (LoadedPackages > 0u);
			}

			if (bIsUserPathsChanged)
			{
				static const FName InstaMATLibraryWindowTabName(TEXT("InstaMATLibraryWindow"));
				TWeakPtr<SDockTab> LibraryTab = FGlobalTabmanager::Get()->FindExistingLiveTab(InstaMATLibraryWindowTabName);

				if (LibraryTab.IsValid())
				{
					const EAppReturnType::Type UserResponse = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("ReOpeningMATLibraryMessage", "The changes that you have made require re-opening the MAT library window to take effect.\nWould you like to re-open the MAT library window now?"), MessageTitle);
					if (UserResponse == EAppReturnType::Type::Yes)
					{
						LibraryTab.Pin()->RequestCloseTab();
						LibraryTab = FGlobalTabmanager::Get()->TryInvokeTab(InstaMATLibraryWindowTabName);
						if (!LibraryTab.IsValid())
						{
							FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ReOpeningMATLibraryFailedMessage", "Failed to re-open MAT library window. Please re-open MAT library window to see the changes."), MessageTitle);
						}
					}
				}
			}
		}
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UInstaMATSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();
	static const FName LockSettingName = TEXT("bLockTextureResolution");

	if (PropertyName.IsEqual(LockSettingName))
	{
		// Apply lock to resolutions (Default and Preview)
		if (bLockTextureResolution)
		{
			ResolutionHeight = ResolutionWidth;
			PreviewResolutionHeight = PreviewResolutionWidth;
		}
	}
	else if (bLockTextureResolution) 
	{
		if (PropertyName.IsEqual(TEXT("ResolutionHeight")))
		{
			ResolutionWidth = ResolutionHeight;
		}
		else if (PropertyName.IsEqual(TEXT("ResolutionWidth")))
		{
			ResolutionHeight = ResolutionWidth;
		}
		else if (PropertyName.IsEqual(TEXT("PreviewResolutionHeight")))
		{
			PreviewResolutionWidth = PreviewResolutionHeight;
		}
		else if (PropertyName.IsEqual(TEXT("PreviewResolutionWidth")))
		{
			PreviewResolutionHeight = PreviewResolutionWidth;
		}
	}

	SaveToDefaultObject();
}

bool UInstaMATSettings::IsCustomVRAMSettingAvailable()
{
	FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>(TEXT("InstaMAT"));
	InstaMAT::IInstaMAT* const InstaMATAPI = InstaMATModule.GetInstaMATAPI();

	const int64 kKiloByte = 1024;
	const int64 TotalMemory = InstaMATAPI->GetTotalAvailableVideoMemory() / kKiloByte / kKiloByte;

	return TotalMemory > UInstaMATSettings::kMinimumMemoryBudget;
}

void UInstaMATSettings::SaveToDefaultObject()
{
	UInstaMATSettings* const DefaultObject = GetMutableDefault<UInstaMATSettings>();
	if (DefaultObject != this)
	{
		DefaultObject->ResolutionWidth = ResolutionWidth;
		DefaultObject->ResolutionHeight = ResolutionHeight;
		DefaultObject->ExecutionFormat = ExecutionFormat;
		DefaultObject->PreviewResolutionWidth = PreviewResolutionWidth;
		DefaultObject->PreviewResolutionHeight = PreviewResolutionHeight;
		DefaultObject->EnvironmentFolder = EnvironmentFolder;
		DefaultObject->UserFolders = UserFolders;
		DefaultObject->Delay = Delay;
		DefaultObject->bIsUnsupportedTypesVisible = bIsUnsupportedTypesVisible;
	}
	DefaultObject->Modify();
	DefaultObject->SaveConfig();
}

bool UInstaMATSettings::IsInstaMATConfigured()
{
	UInstaMATSettings* const DefaultObject = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
	DefaultObject->LoadConfig();
	DefaultObject->EnsureDefaultUserPathIsSet();
	DefaultObject->SaveConfig();

	// Assume that the InstaMAT installation folder is correct if it is present and a valid path
	const FString& EnvironmentFolder = DefaultObject->EnvironmentFolder;

	if (EnvironmentFolder.IsEmpty() || !FPaths::DirectoryExists(EnvironmentFolder))
		return false;

	return true;
}

void UInstaMATSettings::EnsureDefaultUserPathIsSet()
{
	// Remove empty entries
	UserFolders.RemoveAll([](const FInstaMATUserDirectory& Directory) { return Directory.UserPath.Path.IsEmpty(); });

	// Remove doubles
	TSet<int32> DoubleEntries;
	for (int32 Index = 0; Index < UserFolders.Num(); Index++)
	{
		const FInstaMATUserDirectory& UserDirectory = UserFolders[Index];

		for (int32 SearchIndex = Index + 1; SearchIndex < UserFolders.Num(); SearchIndex++)
		{
			const FInstaMATUserDirectory& CompareDirectory = UserFolders[SearchIndex];

			if (CompareDirectory.UserPath.Path.Compare(UserDirectory.UserPath.Path, ESearchCase::IgnoreCase) == 0)
			{
				DoubleEntries.Add(SearchIndex);
			}
		}
	}

	DoubleEntries.Sort([](const int32 LHS, const int32 RHS) { return LHS < RHS; });

	for (const int32 Index : DoubleEntries)
	{
		UserFolders.RemoveAt(Index);
	}

	/// The fnEnsureDefaultUserPathIsSet lambda ensures a valid default user path is set.
	const auto fnEnsureDefaultUserPathIsSet = [this]()
	{
		// Get the documents directory
		const FString UserDirectory = InstaMATDirectoryUtility::GetUserDirectory();

		if (UserDirectory.IsEmpty())
			return;

		FInstaMATUserDirectory Directory;
		Directory.bIsDefault = true;
		Directory.UserPath.Path = UserDirectory;
		UserFolders.Add(Directory);
	};

	if (UserFolders.Num() == 0)
	{
		fnEnsureDefaultUserPathIsSet();
		return;
	}

	bool bIsValidDefaultUserPathSet = false;
	for (FInstaMATUserDirectory& UserDirectory : UserFolders)
	{
		if (!bIsValidDefaultUserPathSet)
		{
			if (!UserDirectory.UserPath.Path.IsEmpty() &&
				UserDirectory.bIsDefault)
			{
				bIsValidDefaultUserPathSet = true;
			}
		}
		else if (!UserDirectory.UserPath.Path.IsEmpty() &&
			UserDirectory.bIsDefault)
		{
			// ensure only a single default path is set.
			UserDirectory.bIsDefault = false;
		}
	}

	if (!bIsValidDefaultUserPathSet)
	{
		// Set the first as default
		check(UserFolders.Num() > 0);
		UserFolders[0].bIsDefault = true;
	}
}

FString UInstaMATSettings::GetApplicationDirectory()
{
	return InstaMATDirectoryUtility::GetApplicationDirectory();
}

void UInstaMATSettings::NotifyRestartIfNecessary()
{
	bool bIsNotifyRestartRequired = true;
	uint32 NewEmptyPathsCount = 0u;
	if (EmptyPathsCount > 0u)
	{
		for (int32 Index = 0; Index < UserFolders.Num(); Index++)
		{
			const FInstaMATUserDirectory& UserDirectory = UserFolders[Index];
			if (!UserDirectory.UserPath.Path.IsEmpty())
				continue;

			NewEmptyPathsCount++;
		}
		// A notify is requried if there is a change that have been made to a non-empty path.
		bIsNotifyRestartRequired = NewEmptyPathsCount >= EmptyPathsCount;
	}
	if (!bIsNotifyRestartRequired)
		return;

	TSharedPtr<SNotificationItem> NotificationPin = RestartNotificationItem.Pin();
	if (NotificationPin.IsValid())
		return;

	FNotificationInfo Info(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_RestartRequired", "The changes that you have made require a restart of the project to take effect."));

	Info.bFireAndForget = false;
	// Set the width so that the notification doesn't resize as its text changes.
	// The same width that is used for the default Unreal Engine notifications.
	Info.WidthOverride = 300.0f;
	Info.bUseLargeFont = false;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = false;
	Info.Image = FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMATUI.TabIcon"));

	// Add Restart Now button.
	Info.ButtonDetails.Add(FNotificationButtonInfo(
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_RestartNow", "Restart Now"),
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_RestartNowToolTip", "Restart now to finish applying your new chagnes."),
		FSimpleDelegate::CreateLambda([this]() {
			TSharedPtr<SNotificationItem> NotificationPin = RestartNotificationItem.Pin();
			if (NotificationPin.IsValid())
			{
				NotificationPin->SetText(NSLOCTEXT(LOCTEXT_NAMESPACE, "RestartingNow", "Restarting..."));
				NotificationPin->SetCompletionState(SNotificationItem::CS_Success);
				NotificationPin->ExpireAndFadeout();
				RestartNotificationItem.Reset();
			}

			const bool bWarn = false;
			FUnrealEdMisc::Get().RestartEditor(bWarn);
		})
	));

	// Add Restart Later button.
	Info.ButtonDetails.Add(FNotificationButtonInfo(
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_RestartLater", "Restart Later"),
		NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_RestartLaterToolTip", "Dismiss this notificaton without restarting. Your changes will not be applied immediately."),
		FSimpleDelegate::CreateLambda([this]() {
			TSharedPtr<SNotificationItem> NotificationPin = RestartNotificationItem.Pin();
			if (NotificationPin.IsValid())
			{
				NotificationPin->SetText(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMAT_RestartDismissed", "Restart Dismissed..."));
				NotificationPin->SetCompletionState(SNotificationItem::CS_None);
				NotificationPin->ExpireAndFadeout();
				RestartNotificationItem.Reset();
			}
		})
	));

	RestartNotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationPin = RestartNotificationItem.Pin();

	if (NotificationPin.IsValid())
	{
		NotificationPin->SetCompletionState(SNotificationItem::CS_Pending);
	}
}

#undef LOCTEXT_NAMESPACE
