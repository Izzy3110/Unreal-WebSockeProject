/**
 * InstaMATModule.cpp (InstaMAT)
 *
 * Copyright 2019-2025 InstaMaterial GmbH - All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * This file and all it's contents are proprietary and confidential.
 *
 * @file InstaMATModule.cpp
 * @copyright 2019-2025 InstaMaterial GmbH. All rights reserved.
 * @section License
 */

#include "InstaMATPCH.h"
#include "InstaMATModule.h"

#include "Runtime/Core/Public/Features/IModularFeatures.h"
#include "ComponentReregisterContext.h"
#include "Slate/InstaMATPluginStyle.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SHyperlink.h"

#include "Framework/Notifications/NotificationManager.h"
#include "EditorSupportDelegates.h"
#include "Interfaces/IPluginManager.h"

#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "HAL/FileManager.h"


#include "InstaMAT/InstaMATSettings.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
#	include "Editor/EditorStyle/Public/EditorStyle.h"
#else
#	include "Editor/EditorStyle/Private/SlateEditorStyle.h"
#endif

#define GetInstaMATPtr()(GInstaMAT.Get())

TUniquePtr<FInstaMAT> GInstaMAT;

InstaMAT::IInstaMAT::pfnForceLicenseRefreshCallback FInstaMATModule::ForceLicenseRefreshDelegate = nullptr;
bool FInstaMATModule::bIsInstaMATFloatingLicenseAvailable = true;

FString InstaMATShared::Version = TEXT("1.0");
FString InstaMATShared::LicenseInformation = TEXT("Unauthorized");

#define LOCTEXT_NAMESPACE "InstaMAT"

#define UE_InstaMAT_LIBRARY_NAME	"InstaMAT"
#define UE_InstaLOD_LIBRARY_NAME	"InstaLOD"

// library file name depends on target platform
#if defined (InstaMAT_LIB_DYNAMIC)
#	if PLATFORM_WINDOWS
#		if PLATFORM_64BITS
#			define InstaMAT_LIB_DYNAMIC_PATH TEXT(UE_InstaMAT_LIBRARY_NAME) TEXT(".dll")
#			define InstaLOD_LIB_DYNAMIC_PATH TEXT(UE_InstaLOD_LIBRARY_NAME) TEXT(".dll")
#		endif
#	elif PLATFORM_MAC
#		define InstaMAT_LIB_DYNAMIC_PATH TEXT("lib" UE_InstaMAT_LIBRARY_NAME ".dylib")
#		define InstaLOD_LIB_DYNAMIC_PATH TEXT("lib" UE_InstaLOD_LIBRARY_NAME ".dylib")
#	elif PLATFORM_LINUX
#		define InstaMAT_LIB_DYNAMIC_PATH TEXT(UE_InstaMAT_LIBRARY_NAME ".so")
#		define InstaLOD_LIB_DYNAMIC_PATH TEXT(UE_InstaLOD_LIBRARY_NAME ".so")
#	endif
#	ifndef InstaMAT_LIB_DYNAMIC_PATH
#		error Platform not supported by InstaMAT
#	endif
#else
#	error InstaMAT supports only dynamically linking the SDK
#endif

#define INSTALOD_LOAD_LIBRARY_MANUAL

#if PLATFORM_MAC
static const FString kInstaMATDylibSubDirectory = TEXT("Contents/Frameworks");		/**< The InstaMAT Studio Dylib subdirectory. */
static const FString kInstaMATContent = TEXT("Contents/");		/**< The InstaMAT Studio Content subdirectory. */
#endif


bool FInstaMATModule::IsInstaMATExecutableInPath(const FString& Path)
{
	if (Path.IsEmpty())
		return false;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		
#if PLATFORM_WINDOWS
	const FString InstaMATStudioExecutable = TEXT("InstaMAT Studio.exe");

	TArray<FString> Files;
	PlatformFile.FindFiles(Files, *Path, TEXT("exe"));

	const FString* Result = Files.FindByPredicate([&InstaMATStudioExecutable](const FString& File)
	{
		return File.EndsWith(InstaMATStudioExecutable, ESearchCase::IgnoreCase);
	});
		
	return Result != nullptr;
		
#elif PLATFORM_MAC
		
	const FString InstaMATStudioExecutable = TEXT("Contents/MacOS/InstaMAT Studio");
	const FString ExecutablePath = FPaths::Combine(Path, InstaMATStudioExecutable);
	return PlatformFile.FileExists(*ExecutablePath);
#endif
}

IInstaMAT* FInstaMATModule::GetInstaMATInterface() 
{
	return GetInstaMATPtr();
}

void FInstaMATModule::StartupModule()
{
	InstaMATAPI = nullptr;
	bIsInstaMATFloatingLicenseAvailable = true;

	// Initialize style
	{
		FInstaMATPluginStyle::Initialize();
		FInstaMATPluginStyle::ReloadTextures();
	}

	// Get settings
	UInstaMATSettings* const Settings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
	Settings->LoadConfig();
	Settings->EnsureDefaultUserPathIsSet();
	Settings->SaveConfig();
	
	// Check if an InstaMAT Studio path is set
	FString EnvironmentPath = Settings->EnvironmentFolder;
	
	/// The fnEnsureInstaMATStudioPathIsSet lambda ensures that a valid InstaMAT Studio directory is set.
	const auto fnEnsureInstaMATStudioPathIsSet = [/*copy:*/ Settings, &EnvironmentPath](bool bEnforceNewPath = false) -> bool
	{
		FString Value = UInstaMATSettings::GetApplicationDirectory();

		if (Value.IsEmpty() || bEnforceNewPath)
		{
			Value = InstaMATShared::OpenInstaMATStudioSelectionWindowModal();

			if (Value.IsEmpty())
				return false;
		}
		
		Settings->EnvironmentFolder = Value;
		Settings->SaveConfig();
		EnvironmentPath = Value;
		return true;
	};

	const static FString InvalidMATPathErrorMessage = TEXT("InstaMAT: No valid InstaMAT Studio installation directory was provided. Please make sure that the selected path is the valid InstaMAT Studio installation directory.");
	if (EnvironmentPath.IsEmpty())
	{
		if (!fnEnsureInstaMATStudioPathIsSet())
		{
			UE_LOG(LogInstaMAT, Fatal, TEXT("%s"), *InvalidMATPathErrorMessage);
			return;
		}
		
		Settings->LoadConfig();
		EnvironmentPath = Settings->EnvironmentFolder;
		
#if PLATFORM_MAC
		EnvironmentPath = FPaths::Combine(EnvironmentPath, kInstaMATDylibSubDirectory);
#endif
	}
#if PLATFORM_MAC
	else
	{
		EnvironmentPath = FPaths::Combine(EnvironmentPath, kInstaMATDylibSubDirectory);
		
		if (!FPaths::DirectoryExists(EnvironmentPath))
		{
			EnvironmentPath = "";
			UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Specified InstaMAT Studio installation directory='%s' is invalid."), *EnvironmentPath);
			
			if (!fnEnsureInstaMATStudioPathIsSet())
			{
				UE_LOG(LogInstaMAT, Fatal, TEXT("%s"), *InvalidMATPathErrorMessage);
				return;
			}
			
			EnvironmentPath = FPaths::Combine(EnvironmentPath, kInstaMATDylibSubDirectory);
		}
	}
#endif

#if defined (InstaMAT_LIB_DYNAMIC) 
	const FString LibraryFileName = InstaMAT_LIB_DYNAMIC_PATH;
	bool bIsInstaMATLoaded = false;
	bool bIsInstaMATVersionValid = false;
	uint32 MinorVersion, MajorVersion;
	
	// Try loading library from environment path
	FString LibraryPath = FPaths::Combine(*EnvironmentPath, LibraryFileName);

	/// The fnGetValidLibraryPath lambda ensures that the library path is valid.
	const auto fnGetValidLibraryPath = [&LibraryPath, &fnEnsureInstaMATStudioPathIsSet, &EnvironmentPath, &LibraryFileName](bool bEnforceNewPath = false) -> bool
	{
			if (bEnforceNewPath)
			{
				LibraryPath = FString();
			}

			while (!FPaths::FileExists(LibraryPath))
			{
				if (!fnEnsureInstaMATStudioPathIsSet(/*bEnforceNewPath:*/true))
				{
					return false;
				}

#if PLATFORM_MAC
				LibraryPath = FPaths::Combine(*EnvironmentPath, kInstaMATDylibSubDirectory, LibraryFileName);
#else
				LibraryPath = FPaths::Combine(*EnvironmentPath, LibraryFileName);
#endif
			}
		return true;
	};

	/// The fnLoadInstaMAT lambda loads InstaMAT.
	const auto fnLoadInstaMAT = [this , &LibraryPath, &bIsInstaMATVersionValid, &bIsInstaMATLoaded, &MinorVersion, &MajorVersion]()
	{
		if (void* pLibraryHandle = FPlatformProcess::GetDllHandle(*LibraryPath))
		{
			// Check DLL version
			if (pfnGetInstaMATBuildDate pGetInstaMATBuildDate = (pfnGetInstaMATBuildDate)FPlatformProcess::GetDllExport(pLibraryHandle, TEXT("GetInstaMATBuildDate")))
			{
				int InstaMATVersion;
				if (pGetInstaMATBuildDate(&InstaMATVersion) != nullptr)
				{
					MinorVersion = InstaMATVersion & 0xFFFF;
					MajorVersion = (InstaMATVersion&(~0xFFFF)) >> 16;
					bIsInstaMATVersionValid = INSTAMAT_API_VERSION == InstaMATVersion;
				}
			}

			if (bIsInstaMATVersionValid)
			{
				if (pfnGetInstaMAT pGetInstaMAT = (pfnGetInstaMAT)FPlatformProcess::GetDllExport(pLibraryHandle, TEXT("GetInstaMAT")))
				{
					if (pGetInstaMAT(INSTAMAT_API_VERSION, &InstaMATAPI) != 0u)
					{
						bIsInstaMATLoaded = true;
						UE_LOG(LogInstaMAT, Log, TEXT("%s"), UTF8_TO_TCHAR(InstaMATAPI->GetBuildDate()));
					}
				}
			}
		}
		else
		{
			const FText Title = FText::FromString(TEXT("InstaMAT"));
			const FText MessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "LibraryLoadingFailureMessageTitle", "Library Loading Failed");
			const FString MessgageContentString = "Failed to load InstaMAT Library, please ensure that InstaMAT application is correctly installed and up to date.";
			InstaMATShared::OpenInstaMATErrorMessageDialog(Title, MessageTitle, FText::FromString(MessgageContentString));
			UE_LOG(LogInstaMAT, Fatal, TEXT("InstaMAT: %s"), *MessgageContentString);
		}
	};

	fnGetValidLibraryPath();
	fnLoadInstaMAT();

	bool bIsPathSelectionValid = true;
	while (!bIsInstaMATVersionValid)
	{
		const FText Title = FText::FromString(TEXT("InstaMAT"));
		const FText MessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "VersionMisMatchMessageTitle", "Version Mismatch");
		FStringFormatNamedArguments FormatArguments;
		FormatArguments.Add(TEXT("InstaMATBuildMajorVerion"), MajorVersion);
		FormatArguments.Add(TEXT("InstaMATBuildMinorVerion"), MinorVersion);
		FormatArguments.Add(TEXT("PluginMajorVerion"), INSTAMAT_API_VERSION_MAJOR);
		FormatArguments.Add(TEXT("PluginMinorVerion"), INSTAMAT_API_VERSION_MINOR);
		
		const FString MessgageContentString = FString::Format(TEXT("The selected InstaMAT application version {InstaMATBuildMajorVerion}.{InstaMATBuildMinorVerion} and the plugin version {PluginMajorVerion}.{PluginMinorVerion} are incompatible.\nPlease make sure that both the InstaMAT application and the plugin files are compatible and up to date."),
			FormatArguments);

		InstaMATShared::OpenInstaMATErrorMessageDialog(Title, MessageTitle, FText::FromString(MessgageContentString));
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: %s"), *MessgageContentString);
		bIsPathSelectionValid = fnGetValidLibraryPath(/*bEnforceNewPath:*/true);
		if (!bIsPathSelectionValid)
			break;

		fnLoadInstaMAT();
	}

	if (!bIsPathSelectionValid)
	{
		UE_LOG(LogInstaMAT, Fatal, TEXT("%s"), *InvalidMATPathErrorMessage);
	}

	if (!bIsInstaMATLoaded)
	{
		const FText Title = FText::FromString(TEXT("InstaMAT"));
		const FText MessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "LibraryLoadingFailureMessageTitle", "Library Loading Failed");
		const FString MessgageContentString = "Unable to load InstaMAT Library. If InstaMAT still fails to load, please update InstaMAT Studio and InstaMAT For Unreal Engine.";
		InstaMATShared::OpenInstaMATErrorMessageDialog(Title, MessageTitle, FText::FromString(MessgageContentString));
		UE_LOG(LogInstaMAT, Fatal, TEXT("InstaMAT: %s"), *MessgageContentString);
	}
	
#else
	GetInstaMAT(InstaMAT_API_VERSION, &InstaMATAPI);
	UE_LOG(LogInstaMAT, Display, TEXT("%s"), UTF8_TO_TCHAR(InstaMATAPI->GetBuildDate()));
#endif
	
	GInstaMAT.Reset(FInstaMAT::Create(InstaMATAPI));
	
	InstaMATAPI->SetLicenseUnavailableCallback(&FInstaMATModule::LicenseUnavailableCallback);
	if (!InstaMATAPI->InitializeAuthorization("InstaMAT", nullptr))
	{
		UE_LOG(LogInstaMAT, Fatal, TEXT("InstaMAT: Failed to initialize InstaMAT authorization module with error: %s"), UTF8_TO_TCHAR(InstaMATAPI->GetAuthorizationInformation()));
	}

	if (!InstaMATAPI->IsHostAuthorized())
	{
		InstaMATShared::OpenAuthorizationWindowModal();
	}

	// Initialize menu and set static information
	if (GInstaMAT != nullptr)
	{
		InstaMATShared::Version = UTF8_TO_TCHAR(InstaMATAPI->GetBuildDate());
		InstaMATShared::LicenseInformation = UTF8_TO_TCHAR(InstaMATAPI->GetAuthorizationInformation());
		
		UE_LOG(LogInstaMAT, Log, TEXT("%s"), *InstaMATShared::LicenseInformation);
		
		InstallHooks();
	}
	else
	{
		UE_LOG(LogInstaMAT, Fatal, TEXT("InstaMAT Module could not be created"));
		return;
	}

	if (!GInstaMAT->Initialize())
	{
		UE_LOG(LogInstaMAT, Fatal, TEXT("InstaMAT could not be initialized."));
		return;
	}

	check (!Settings->EnvironmentFolder.IsEmpty())

	// Set memory budget
	{
		int64 MemoryBudget = Settings->VRAMBudget;
		
		if (Settings->bEnableCustomMemorySetting && MemoryBudget < 0)
		{
			// Ensure default value is set if enabled
			Settings->VRAMBudget = GInstaMAT->GetDefaultVRAMBudget();
			Settings->SaveConfig();
			MemoryBudget = Settings->VRAMBudget;
		}
		
		InstaMATAPI->SetVideoMemoryBudget(MemoryBudget);
	}
	
	// Load environment
	{
#if PLATFORM_MAC
		if (EnvironmentPath.Contains(kInstaMATDylibSubDirectory))
		{
			EnvironmentPath.ReplaceInline(*kInstaMATDylibSubDirectory, *kInstaMATContent);
		}
		else
		{
			EnvironmentPath = FPaths::Combine(EnvironmentPath, kInstaMATContent);
		}
#endif
		
		const uint32 LoadedPackages = GInstaMAT->LoadEnvironmentPackageFromPath(EnvironmentPath, /*bIsSystemLibrary:*/ true);
		UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Loaded %u packages from='%s'"), LoadedPackages, *EnvironmentPath);
	}

	// Load user folders
	if (Settings->UserFolders.Num() > 0)
	{
		for (const FInstaMATUserDirectory& Directory : Settings->UserFolders)
		{
			if (Directory.UserPath.Path.IsEmpty())
				continue;

			const uint32 LoadedPackages = GInstaMAT->LoadEnvironmentPackageFromPath(Directory.UserPath.Path, /*bIsSystemLibrary:*/ false);
			UE_LOG(LogInstaMAT, Warning, TEXT("InstaMAT: Loaded %u packages from='%s'"), LoadedPackages, *(Directory.UserPath.Path));
		}

		Settings->ReloadExternalAssets(/*bDisplaySuccessMessage:*/ false);
	}
}

void FInstaMATModule::ShutdownModule()
{
	if (GInstaMAT != nullptr)
	{
		GInstaMAT->Shutdown();
		GInstaMAT = nullptr;
	}

	ForceLicenseRefreshDelegate = nullptr;
	FInstaMATPluginStyle::Shutdown();
}

void FInstaMATModule::InstallHooks()
{
	LateHooksDelegateHandle = FEditorSupportDelegates::UpdateUI.AddLambda([this]() { this->InstallHooksLate(); } );
}

void FInstaMATModule::InstallHooksLate()
{	
	FEditorSupportDelegates::UpdateUI.Remove(LateHooksDelegateHandle);
	
	if (!GInstaMAT->GetInstaMAT()->IsHostAuthorized())
	{
		UE_LOG(LogInstaMAT, Log, TEXT("InstaMAT: Machine not authorized: %s"), UTF8_TO_TCHAR(GInstaMAT->GetInstaMAT()->GetAuthorizationInformation()));
		InstaMATShared::OpenAuthorizationWindowModal();
	}
}

bool FInstaMATModule::IsInitialized()
{
	return InstaMATAPI != nullptr;
}

FString FInstaMATModule::GetDefaultPathForContentBrowser(ELastDirectory::Type ContentBrowserType, const FString& FallbackPath /*= FString()*/)
{
	// Determine the starting path. Try to use the most recently used directory first.
	FString OutputPath;
	{
		FString LastUsedDirectory = FEditorDirectories::Get().GetLastDirectory(ContentBrowserType);

		// Ensure trailing "/" for directory name since TryConvertFilenameToLongPackageName expects one
		if (!LastUsedDirectory.IsEmpty() && !LastUsedDirectory.EndsWith("/"))
		{
			LastUsedDirectory.AppendChar(TEXT('/'));
		}

		if (LastUsedDirectory.IsEmpty() || !FPackageName::TryConvertFilenameToLongPackageName(LastUsedDirectory, OutputPath))
		{
			// No saved path, use the fallback directory.
			OutputPath = FallbackPath;
		}

		// Content browser windows expects no trailing "/" so remove if necessary
		OutputPath.RemoveFromEnd(TEXT("/"));
	}

	return OutputPath;
}

void FInstaMATModule::LicenseUnavailableCallback(InstaMAT::IInstaMAT::pfnForceLicenseRefreshCallback ForceLicenseRefreshCallback)
{
	/// The fnHandleUnavilableFloatingLicense lambda spawns a Dialog modal window for allowing for the user to know that its floating license is currently unavailable. It can only be called in the Game Thread.
	const auto& fnHandleUnavilableFloatingLicense = [](InstaMAT::IInstaMAT::pfnForceLicenseRefreshCallback ForceLicenseRefreshCallback) 
	{
		check(IsInGameThread());

		FInstaMATModule& InstaMATModule = FModuleManager::LoadModuleChecked<FInstaMATModule>("InstaMAT");
		InstaMATModule.bIsInstaMATFloatingLicenseAvailable = false;
		InstaMATModule.ForceLicenseRefreshDelegate = ForceLicenseRefreshCallback;

		UInstaMATSettings::RestartAllInstaMATWindows();
		InstaMATShared::OpenFloatingLicenseUnavailableWindowModal(ForceLicenseRefreshCallback);
	};

	if (IsInGameThread())
	{
		// If in the game thread, immediately call the lambda and display the modal window.
		fnHandleUnavilableFloatingLicense(ForceLicenseRefreshCallback);
	}
	else
	{
		// Schedule a the lambda to be executed in the game thread.
		FFunctionGraphTask::CreateAndDispatchWhenReady([/*copy:*/ ForceLicenseRefreshCallback, fnHandleUnavilableFloatingLicense]()
			{ fnHandleUnavilableFloatingLicense(ForceLicenseRefreshCallback); }, TStatId(), /*InPrerequisites:*/ nullptr, ENamedThreads::GameThread);
	}
}

class SInstaMATDialogWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInstaMATDialogWidget) :
		_OkButtonText(NSLOCTEXT(LOCTEXT_NAMESPACE, "OK", "OK")),
		_CancelButtonText(NSLOCTEXT(LOCTEXT_NAMESPACE, "Cancel", "Cancel"))
	{
	}

	SLATE_ATTRIBUTE(FText, OkButtonText)			/**< The text to be displayed in the "Ok" button. "Ok" by default. */
	SLATE_ATTRIBUTE(FText, CancelButtonText)		/**< The text to be displayed in the "Cancel" button. "Cancel" by default. */
	SLATE_ATTRIBUTE(bool, ShowCancelButton)			/**< Whether the Cancel button is visible or not. */
	SLATE_DEFAULT_SLOT(FArguments, Content)			/**< The content to display in the dialog widget. */
	SLATE_EVENT(FOnClicked, OnOkButtonClicked)		/**< The On Clicked event for the "OK" button. */
	SLATE_EVENT(FOnClicked, OnCancelButtonClicked)	/**< The On Clicked event for the "Cancel" button. Only triggered when ShowCancelButton is True */
	SLATE_END_ARGS()
	
	void Construct( const FArguments& InArgs )
	{
		TSharedPtr< SScrollBox > ScrollBox;

		const bool bIsCancelButtonVisible = InArgs._ShowCancelButton.Get();
		const FOnClicked OkButtonClickedHandler = InArgs._OnOkButtonClicked;
		const FOnClicked CancelButtonClickedHandler = InArgs._OnCancelButtonClicked;
		
		this->ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(300)
			[
				SAssignNew(ScrollBox, SScrollBox)
			]
		 
			+SVerticalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoHeight()
			.Padding(0, 2, 0, 0)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.AutoWidth()
				[
					SNew(SButton)
						.Text(InArgs._OkButtonText)
						.OnClicked_Lambda([/*copy:*/OkButtonClickedHandler, this]()
						{
							FReply ReplyOutput = FReply::Handled();
							if (OkButtonClickedHandler.IsBound())
							{
								ReplyOutput = OkButtonClickedHandler.Execute();
							}

							check(MyWindow != nullptr);
							check(MyWindow.IsValid());

							MyWindow.Pin()->RequestDestroyWindow();
							return ReplyOutput;
						})
				]
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.AutoWidth()
				[
					SNew(SButton)
						.Visibility(bIsCancelButtonVisible ? EVisibility::Visible : EVisibility::Collapsed)
						.Text(InArgs._CancelButtonText)
						.OnClicked_Lambda([/*copy:*/CancelButtonClickedHandler, this]()
						{
							FReply ReplyOutput = FReply::Handled();
							if (CancelButtonClickedHandler.IsBound())
							{
								ReplyOutput = CancelButtonClickedHandler.Execute();
							}

							check(MyWindow != nullptr);
							check(MyWindow.IsValid());
							
							MyWindow.Pin()->RequestDestroyWindow();
							return ReplyOutput;
						})
				]
			]
		 ];
		
		ScrollBox->AddSlot()
		[
			InArgs._Content.Widget
		];
	}
	
	/** Sets the window of this dialog. */
	void SetWindow( TSharedPtr<SWindow> InWindow )
	{
		MyWindow = InWindow;
	}
	
	UNREALED_API static void OpenDialog(const FText& InDialogTitle, const TSharedRef< SWidget >& DisplayContent)
	{
		TSharedPtr<SWindow> Window;
		TSharedPtr<SInstaMATDialogWidget> InstaMATDialogWidget;
		
		Window = SNew(SWindow)
		.Title(InDialogTitle)
		.SizingRule( ESizingRule::Autosized )
		.SupportsMaximize(false) .SupportsMinimize(false)
		[
			SNew(SBorder)
			.Padding(4.0f)
			.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
			[
				SAssignNew(InstaMATDialogWidget, SInstaMATDialogWidget)
				[
					DisplayContent
				]
			]
		];
		
		InstaMATDialogWidget->SetWindow(Window);
		FSlateApplication::Get().AddWindow( Window.ToSharedRef() );
	}
	
private:
	TWeakPtr<SWindow> MyWindow; /**< Pointer to the containing window. */
};

void InstaMATShared::OpenAuthorizationWindowModal()
{
	TSharedRef<SEditableText> UsernameEditableText = SNew(SEditableText);
	TSharedRef<SEditableText> PasswordEditableText = SNew(SEditableText).IsPassword(true);
	
	const FText MessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorization", "InstaMAT: Machine Authorization");
	const FText MessageContent = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationMessage", "This machine is not authorized for InstaMAT.\nPlease enter your license data to authorize this machine.\n\nPlease obtain a valid license or remove InstaMAT from your project.");
	const FText MessageFooter = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationFooter", "Visit http://www.InstaMaterial.com or contact hello@InstaMAT.com\nfor information on how to obtain a valid InstaMAT license.");

	TSharedPtr<SWindow> Window = SNew(SWindow)
	.Title(MessageTitle)
	.SizingRule(ESizingRule::Autosized)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.HasCloseButton(false);

	/// The fnAuthorizeMachineClick lambda handles authorization clicks.
	const auto fnAuthorizeMachineClick = [/*copy:*/UsernameEditableText, /*copy:*/PasswordEditableText, /*copy:*/Window]()
	{
		const FString Username = UsernameEditableText->GetText().ToString();
		const FString Password = PasswordEditableText->GetText().ToString();
		
		if (!GInstaMAT->GetInstaMAT()->AuthorizeMachine(TCHAR_TO_UTF8(*Username), TCHAR_TO_UTF8(*Password)))
		{
			// NOTE: the authorization information contains information about the error
			InstaMATShared::LicenseInformation = UTF8_TO_TCHAR(GInstaMAT->GetInstaMAT()->GetAuthorizationInformation());
			
			const FText ErrorMessageContents = FText::FromString(InstaMATShared::LicenseInformation);
			const FText ErrorMessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAcquiredLicenseFailTitle", "InstaMAT: Failed to acquire license");
			FMessageDialog::Open(EAppMsgType::Ok, ErrorMessageContents, ErrorMessageTitle);
		}
		else
		{
			// NOTE: the authorization information contains information about the error
			InstaMATShared::LicenseInformation = UTF8_TO_TCHAR(GInstaMAT->GetInstaMAT()->GetAuthorizationInformation());
			
			const FText SuccessMessageContents = FText::Format(FText::FromString(TEXT("{0}\n")), FText::FromString(InstaMATShared::LicenseInformation));
			const FText SuccessMessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAcquiredLicenseSuccessTitle", "InstaMAT: Acquired license");
			FMessageDialog::Open(EAppMsgType::Ok, SuccessMessageContents, SuccessMessageTitle);
			if(Window.IsValid())
			{
				Window->RequestDestroyWindow();
			}
		}
		
		return FReply::Handled();
	};

	/// The fnCancelAuthorizationClick lambda handles cancel authorization clicks.
	const auto fnCancelAuthorizationClick = [/*copy:*/Window]()
	{
		if(Window.IsValid())
		{
			Window->RequestDestroyWindow();
		}
		return FReply::Handled();
	};
	
	const TSharedRef<SWidget> DisplayContent =
	SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.AutoHeight()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.Padding(10.0f)
	[
		// InstaMAT logo
		SNew(SBox)
		.WidthOverride(384.0f)
		.HeightOverride(75.75f)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				SNew(SImage)
				.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMAT.LogoMedium")))
			]
		]
	]
	
	+SVerticalBox::Slot()
	.Padding(0.0f, 8.0f, 0.0f, 0.0f)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Justification(ETextJustify::Center)
		.Text(MessageContent)
	]
	
	+SVerticalBox::Slot()
	.Padding(0, 20, 0, 0)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationUserEmail", "Username or Email"))
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBorder)
		.Padding(5.0f)
		.BorderImage(FAppStyle::GetBrush(TEXT("Menu.Background")))
		[
			UsernameEditableText
		]
	]
	
	+SVerticalBox::Slot()
	.Padding(0, 10, 0, 0)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationPassword", "Password or License Key"))
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBorder)
		.Padding(5.0f)
		.BorderImage(FAppStyle::GetBrush(TEXT("Menu.Background")))
		[
			PasswordEditableText
		]
	]
	
	+SVerticalBox::Slot()
	.Padding(0, 20, 0, 20)
	.HAlign(HAlign_Center)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Justification(ETextJustify::Center)
		.Text(MessageFooter)
	]

	+SVerticalBox::Slot()
	.HAlign(HAlign_Right)
	.AutoHeight()
	.Padding(0.0f, 10.0f, 0.0f, 0.0f)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationAuthorize", "Authorize"))
			.OnClicked_Lambda(fnAuthorizeMachineClick)
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationCancel", "Cancel"))
			.OnClicked_Lambda(fnCancelAuthorizationClick)
		]
	];
	
	Window->SetContent
	(
		SNew(SBorder)
		.Padding(10)
		.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
		[
			DisplayContent
		]
	);
	
	GEditor->EditorAddModalWindow(Window.ToSharedRef());
	
	if (!GInstaMAT->GetInstaMAT()->IsHostAuthorized())
	{
		if (FMessageDialog::Open(EAppMsgType::OkCancel, MessageContent, MessageTitle) == EAppReturnType::Ok)
		{
			// Is authorized now
		}
		else
		{
			// Restart the authorization loop
			OpenAuthorizationWindowModal();
		}
	}
	else
	{
		const FText AuthorizedMessageContent = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationCompleted", "A valid InstaMAT license has been acquired for this machine.\n\nInstaMAT will automatically refresh the license if necessary.");
		FMessageDialog::Open(EAppMsgType::Ok, AuthorizedMessageContent, MessageTitle);
	}
}

void InstaMATShared::OpenDeauthorizationWindowModal()
{
	const FText MessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATDeauthorizeNoLicense", "InstaMAT: Deauthorize");
	
	if (!GInstaMAT->GetInstaMAT()->IsHostAuthorized())
	{
		FText ErrorMessageContents = FText::FromString(InstaMATShared::LicenseInformation);
		FMessageDialog::Open(EAppMsgType::Ok, ErrorMessageContents, MessageTitle);
		return;
	}
	
	TSharedRef<SEditableText> UsernameEditableText = SNew(SEditableText);
	TSharedRef<SEditableText> PasswordEditableText = SNew(SEditableText).IsPassword(true);
	
	const FText MessageContent = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATDeauthorizationMessage", "Please enter your license data to deauthorize this machine.\n");
	const FText MessageFooter = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATDeauthorizationFooter", "Visit http://www.InstaMaterial.com or contact hello@InstaMAT.com\nfor information on how to obtain a valid InstaMAT license.");
	
	/// The fnDeauthorizeMachineClick lambda handles deauthorization clicks.
	const auto fnDeauthorizeMachineClick = [UsernameEditableText, PasswordEditableText, MessageTitle]()
	{
		const FString Username = UsernameEditableText->GetText().ToString();
		const FString Password = PasswordEditableText->GetText().ToString();
		
		if (!GInstaMAT->GetInstaMAT()->DeauthorizeMachine(TCHAR_TO_UTF8(*Username), TCHAR_TO_UTF8(*Password)))
		{
			// NOTE: The authorization information contains information about the error
			InstaMATShared::LicenseInformation = UTF8_TO_TCHAR(GInstaMAT->GetInstaMAT()->GetAuthorizationInformation());
			
			const FText ErrorMessageContents = FText::FromString(InstaMATShared::LicenseInformation);
			const FText ErrorMessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATDeauthorizationFailedTitle", "InstaMAT: Deauthorization failed");
			FMessageDialog::Open(EAppMsgType::Ok, ErrorMessageContents, ErrorMessageTitle);
		}
		else
		{
			const FText SuccessMessageContents = FText::Format(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATDeauthorized", "InstaMAT license removed and machine deauthorized.\n"), FText::FromString(InstaMATShared::LicenseInformation));
			
			FMessageDialog::Open(EAppMsgType::Ok, SuccessMessageContents, MessageTitle);
		}
		
		return FReply::Handled();
	};
	
	const TSharedRef<SWidget> DisplayContent =
	SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.AutoHeight()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.Padding(10.0f)
	[
		// InstaMAT logo
		SNew(SScaleBox)
		.Stretch(EStretch::None)
		.OverrideScreenSize(FVector2D(256.0, 50.5))
		[
			SNew(SImage)
				.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMAT.LogoMedium")))
		]
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Justification(ETextJustify::Center)
		.Text(MessageContent)
	]
	+SVerticalBox::Slot()
	.Padding(0.0f, 20.0f, 0.0f, 0.0f)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationUserEmail", "Username or Email"))
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBorder)
		.Padding(5.0f)
		.BorderImage(FAppStyle::GetBrush(TEXT("Menu.Background")))
		[
			UsernameEditableText
		]
	]
	+SVerticalBox::Slot()
	.Padding(0.0f, 10.0f, 0.0f, 0.0f)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationPassword", "Password or License Key"))
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBorder)
		.Padding(5.0f)
		.BorderImage(FAppStyle::GetBrush(TEXT("Menu.Background")))
		[
			PasswordEditableText
		]
	]
	+SVerticalBox::Slot()
	.HAlign(HAlign_Right)
	.AutoHeight()
	.Padding(0.0f, 10.0f, 0.0f, 0.0f)
	[
		SNew(SButton)
		.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorizationDeauthorize", "Deauthorize") )
		.OnClicked_Lambda(fnDeauthorizeMachineClick)
	]
	+SVerticalBox::Slot()
	.Padding(0.0f, 20.0f, 0.0f, 20.0f)
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(MessageFooter)
	];
	
	TSharedRef<SInstaMATDialogWidget> InstaMATDialogWidget =
	SNew(SInstaMATDialogWidget)
	.ShowCancelButton(false)
	[
		DisplayContent
	];
	
	TSharedPtr<SWindow> Window = SNew(SWindow)
	.Title(MessageTitle)
	.SizingRule(ESizingRule::Autosized)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	[
		SNew( SBorder )
		.Padding(10.0f)
		.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
		[
			InstaMATDialogWidget
		]
	 ];
	InstaMATDialogWidget->SetWindow(Window);
	
	GEditor->EditorAddModalWindow(Window.ToSharedRef());
}

FString InstaMATShared::OpenInstaMATStudioSelectionWindowModal()
{
	TSharedRef<SEditableText> UsernameEditableText = SNew(SEditableText);
	TSharedRef<SEditableText> PasswordEditableText = SNew(SEditableText).IsPassword(true);

	const FText MessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATSelectStudioTitle", "Select InstaMAT Studio");
	const FText MessageContent = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATSelectStudioMessage", "Please select the InstaMAT Studio installation path.");

	EAppReturnType::Type Result = EAppReturnType::Cancel;

	const FString kInstaMATURL = TEXT("https://cloud.InstaMAT.io/");

	const TSharedRef<SWidget> DisplayContent =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(10.0f, 10.0f, 10.0f, 20.0f)
		[
			// InstaMAT logo
			SNew(SImage)
				.Image(FInstaMATPluginStyle::Get().GetBrush(TEXT("InstaMAT.LogoTiny")))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text(MessageContent)
		]

		+ SVerticalBox::Slot()
		.Padding(0.0f, 10.0f, 0.0f, 20.0f)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
					.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATPath", "If InstaMAT Studio is not installed on this workstation please download from the "))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SHyperlink)
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATLicenseWebApp", "InstaMAT License Management Web App"))
				.OnNavigate_Lambda([/*copy:*/ kInstaMATURL]()
					{
						FPlatformProcess::LaunchURL(*kInstaMATURL, nullptr, nullptr);
					})
			]
		];

	TSharedRef<SInstaMATDialogWidget> InstaMATDialogWidget =
		SNew(SInstaMATDialogWidget)
		.ShowCancelButton(true)
		.OkButtonText(NSLOCTEXT(LOCTEXT_NAMESPACE, "SelectInstaMATPath", "Select InstaMAT Studio path"))
		.OnOkButtonClicked_Lambda([&Result]() -> FReply
			{
				Result = EAppReturnType::Ok;
				return FReply::Handled();
			})
		[
			DisplayContent
		];

	TSharedPtr<SWindow> Window = SNew(SWindow)
	.Title(MessageTitle)
	.SizingRule(ESizingRule::Autosized)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	[
		SNew(SBorder)
			.Padding(10.0f)
			.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
			[
				InstaMATDialogWidget
			]
	];
	InstaMATDialogWidget->SetWindow(Window);

	GEditor->EditorAddModalWindow(Window.ToSharedRef());

	if (Result == EAppReturnType::Cancel)
		return FString();

	IDesktopPlatform* const Platform = FDesktopPlatformModule::Get();

	if (Platform == nullptr)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: Failed to retrieve the desktop module."));
		return FString();
	}

	const FString ApplicationDirectory = UInstaMATSettings::GetApplicationDirectory();
	FString Path;

	const FString Title(TEXT("Select InstaMAT Studio"));
#if PLATFORM_WINDOWS
	if (!Platform->OpenDirectoryDialog(nullptr, Title, /*DefaultFolder:*/ ApplicationDirectory, Path))
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: InstaMAT Studio path not selected, please ensure that InstaMAT Studio is installed on your workstation to run this plugin."));
		return FString();
	}
#elif PLATFORM_MAC
	
	TArray<FString> Paths;

	// NOTE: The file extension type "*.app" is not currently working on latest macOS version. Using a generic file type "*.*" in the meantime so the user can select the InstaMAT Studio application.
	if (!Platform->OpenFileDialog(nullptr, Title, /*DefaultFolder:*/ ApplicationDirectory, /*DefaultFile*/FString(TEXT("InstaMAT Studio")), /*FileTypes:*/ FString(TEXT("Application|*.*")), EFileDialogFlags::None, Paths) || Paths.Num() == 0)
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: InstaMAT Studio path not selected, please ensure that InstaMAT Studio is installed on your workstation to run this plugin."));
		return FString();
	}
	
	Path = Paths[0];
#endif

	if (!FInstaMATModule::IsInstaMATExecutableInPath(Path))
	{
		UE_LOG(LogInstaMAT, Error, TEXT("InstaMAT: The selected directory='%s' does not contain the InstaMAT Studio executable."), *Path);
		return OpenInstaMATStudioSelectionWindowModal();
	}

	return Path;
}

void InstaMATShared::OpenInstaMATErrorMessageDialog(const FText& Title, const FText& MessageTitle,const FText& MessageContent)
{
	check(!Title.IsEmpty() && !MessageTitle.IsEmpty() && !MessageContent.IsEmpty());

	static const FName Icon = "MessageLog.Error";
	TWeakPtr<SWindow> WindowWeakPtr;

	TSharedPtr<SWindow> Window = SNew(SWindow)
	.Title(Title)
	.SizingRule(ESizingRule::Autosized)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	[
		SNew(SBorder)
		.Padding(10.0f)
		.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(0.0f, 0.0f, 2.0f, 0.0f)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush(Icon))
				]
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(MessageTitle)
					.AutoWrapText(true)
				]
			]
			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(MessageContent)
				.AutoWrapText(true)
			]
			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "OkButtonText", "Ok"))
				.OnClicked_Lambda([&WindowWeakPtr]()->FReply
				{
					if(WindowWeakPtr.IsValid())
					{
						WindowWeakPtr.Pin()->RequestDestroyWindow();
					}
					return FReply::Handled();
				})
			]
		]
	];
	WindowWeakPtr = Window.ToWeakPtr();
	GEditor->EditorAddModalWindow(Window.ToSharedRef());
}

void InstaMATShared::OpenFloatingLicenseUnavailableWindowModal(InstaMAT::IInstaMAT::pfnForceLicenseRefreshCallback ForceLicenseRefreshCallback)
{
	const FText MessageTitle = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATAuthorization", "InstaMAT License Unavailable");
	const FText MessageContent = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATFloatingLicenseUnavailable", "You have authorized InstaMAT with a floating license, but the license could not be verified by the Abstract servers.\n\nTo continue using InstaMAT, please make sure that you're connected to the internet and that your license is not in use on another computer.");
	const FText TryAgainButtonText = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATFloatingLicenseUnavailableTryAgainButtonText", "Try again");
	const FText QuitButtonText = NSLOCTEXT(LOCTEXT_NAMESPACE, "InstaMATFloatingLicenseUnavailableQuit", "Quit");

	const static FWindowStyle WindowStyle = FInstaMATPluginStyle::Get().GetWidgetStyle<FWindowStyle>("InstaMAT.FloatingLicense.Window.Style");

	TSharedPtr<SWindow> Window = SNew(SWindow)
		.Style(&WindowStyle)
		.CreateTitleBar(false)
		.SizingRule(ESizingRule::Autosized)
		.SupportsMaximize(false)
		.HasCloseButton(false)
		.SupportsMinimize(false);

	TWeakPtr<SWindow> WindowWeakPtr = Window.ToWeakPtr();

	Window->SetContent(
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.Padding(20.0f, 0.0f)
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(0.0f, 35.0f, 0.0f, 40.0f)
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						//NOTE: we use the text instead of logo as we show the window so early that UE does not render images at that stage. We need to decide at which stage we set the LicenseUnavailableCallback so that UE can render the images (InstallHooksLate).
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(MessageTitle)
								.TextStyle(FInstaMATPluginStyle::Get(), "InstaMAT.FloatingLicense.Window.Title")
						]
				]
				+ SVerticalBox::Slot()
				.Padding(0.0f, 0.0f, 0.0f, 20.0f)
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(MessageContent)
						.ColorAndOpacity(FLinearColor::White)
				]
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 20.0f)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "NoBorder")
								.Content()
								[
									SNew(SOverlay)
										+ SOverlay::Slot()
										[
											SNew(SImage)
												.Image_Lambda([]() -> const FSlateBrush* {
												return FInstaMATPluginStyle::Get().GetBrush("InstaMAT.FloatingLicense.Button.Normal");
													})
										]
										+ SOverlay::Slot()
										.Padding(20.0f, 10.0f)
										[
											SNew(SHorizontalBox)
												+ SHorizontalBox::Slot()
												.HAlign(HAlign_Center)
												.AutoWidth()
												[
													SNew(STextBlock)
														.Text(TryAgainButtonText)
														.Justification(ETextJustify::Right)
												]
										]
								]
								.OnClicked_Lambda([/*copy:*/WindowWeakPtr]()
									{
										UInstaMATSettings* const Settings = UInstaMATSettings::StaticClass()->GetDefaultObject<UInstaMATSettings>();
										if (Settings->TryFloatingLicenseAgain())
										{
											if (const TSharedPtr<SWindow> Window = WindowWeakPtr.Pin())
											{
												Window->RequestDestroyWindow();
											}
										}
										return FReply::Handled();
									})
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "NoBorder")
								.Content()
								[
									SNew(SOverlay)
										+ SOverlay::Slot()
										[
											SNew(SImage)
												.Image_Lambda([]() -> const FSlateBrush* {
												return FInstaMATPluginStyle::Get().GetBrush("InstaMAT.FloatingLicense.Button.Normal");
													})
										]
										+ SOverlay::Slot()
										.Padding(20.0f, 10.0f)
										[
											SNew(SHorizontalBox)
												+ SHorizontalBox::Slot()
												.HAlign(HAlign_Center)
												.AutoWidth()
												[
													SNew(STextBlock)
														.Text(QuitButtonText)
														.Justification(ETextJustify::Right)
												]
										]
								]
								.OnClicked_Lambda([/*copy:*/WindowWeakPtr, /*copy:*/MessageContent]()
									{
										if (const TSharedPtr<SWindow> Window = WindowWeakPtr.Pin())
										{
											Window->RequestDestroyWindow();
										}

										UE_LOG(LogInstaMAT, Fatal, TEXT("%s"), *MessageContent.ToString());
										return FReply::Handled();
									})
						]
				]
		]
	);

	GEditor->EditorAddModalWindow(Window.ToSharedRef());
}

IMPLEMENT_MODULE(FInstaMATModule, InstaMAT);

#undef LOCTEXT_NAMESPACE
