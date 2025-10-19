// Copyright Athian Games. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MySQL : ModuleRules
{
    private string ThirdPartyPath => Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/"));

    public string ProjectBinariesPath =>
        Path.Combine(Target.ProjectFile!.Directory.FullName, "Binaries", Target.Platform.ToString());

    private void CopyToBinaries(string filePath, string destination, ReadOnlyTargetRules target)
    {
        _ = target; // explicitly mark as intentionally unused

        var fileName = Path.GetFileName(filePath);
        var destPath = Path.Combine(destination, fileName);

        if (!Directory.Exists(destination))
            Directory.CreateDirectory(destination);

        if (!File.Exists(destPath))
            File.Copy(filePath, destPath, true);

        RuntimeDependencies.Add(destPath);
        PublicDelayLoadDLLs.Add(destPath);
    }

    public MySQL(ReadOnlyTargetRules target) : base(target)
    {
        // Core include paths and dependencies
        PrivateIncludePaths.AddRange(["MySql/Private"]);

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange([
            "Core", "CoreUObject", "Engine", "RHI",
            "ImageWrapper", "RenderCore", "ImageWriteQueue",
            "InputCore", "Projects"
        ]);

        PrivateDependencyModuleNames.AddRange([
            "XmlParser", "Core", "ImageWrapper", "Engine"
        ]);

        // Supported platforms only
        if (target.Platform != UnrealTargetPlatform.Win64 &&
            target.Platform != UnrealTargetPlatform.LinuxArm64 &&
            target.Platform != UnrealTargetPlatform.Linux)
        {
            return;
        }

        var platformString = "";
        var libExtension = "";
        var binExtension = "";

        if (target.Platform == UnrealTargetPlatform.Win64)
        {
            platformString = "Win64";
            libExtension = ".lib";
            binExtension = ".dll";
            PublicDefinitions.Add("NTDDI_WIN7SP1");
        }
        else if (target.Platform == UnrealTargetPlatform.LinuxArm64)
        {
            platformString = "Linux-arm64";
            libExtension = ".so";
            binExtension = ".so";
        }
        else if (target.Platform == UnrealTargetPlatform.Linux)
        {
            platformString = "Linux-x64";
            libExtension = ".so";
            binExtension = ".so";
        }

        var mySqlPath = Path.Combine(ThirdPartyPath, "MariaDB");
        var mySqlLibraryPath = Path.Combine(mySqlPath, "lib", platformString);
        var binariesDir = ProjectBinariesPath;
        var pluginPath = Path.Combine(mySqlLibraryPath, "plugin");

        PublicIncludePaths.Add(Path.Combine(mySqlPath, "include"));
        PublicSystemLibraryPaths.Add(mySqlLibraryPath);

        // Add static and dynamic libraries
        foreach (var file in Directory.GetFiles(mySqlLibraryPath, "*" + libExtension))
            PublicAdditionalLibraries.Add(file);

        foreach (var file in Directory.GetFiles(mySqlLibraryPath, "*" + binExtension))
            CopyToBinaries(file, binariesDir, target);

        foreach (var file in Directory.GetFiles(pluginPath, "*" + libExtension))
            PublicAdditionalLibraries.Add(file);

        foreach (var file in Directory.GetFiles(pluginPath, "*" + binExtension))
            CopyToBinaries(file, binariesDir, target);
    }
}
