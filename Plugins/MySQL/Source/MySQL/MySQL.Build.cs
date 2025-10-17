// Copyright Athian Games. All Rights Reserved. 

using UnrealBuildTool;
using System.IO;


public class MySQL : ModuleRules
{

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    public string ProjectBinariesPath
    {

        get { return Path.Combine(Target.ProjectFile!.Directory.FullName, "Binaries", Target.Platform.ToString()); }
    }

    private void CopyToBinaries(string FilePath, string destination, ReadOnlyTargetRules Target)
    {
        
        string Filename = Path.GetFileName(FilePath);
        string DestPath = Path.Combine(destination, Filename);

        if (!Directory.Exists(destination))
        {
            Directory.CreateDirectory(destination);
        }

        if (!File.Exists(DestPath))
        {
            File.Copy(FilePath, DestPath, true);
        }
        
        RuntimeDependencies.Add(DestPath);
        PublicDelayLoadDLLs.Add(Path.Combine(DestPath));

    }
    public MySQL(ReadOnlyTargetRules Target) : base(Target)
    {

        PrivateIncludePaths.AddRange(new string[]
        {
            "MySQL/Private"

        });

    
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        UndefinedIdentifierWarningLevel = WarningLevel.Off;
        bEnableExceptions = true;
        
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "RHI",
            "ImageWrapper", "RenderCore", "ImageWriteQueue", "InputCore" , "Projects" });
        PrivateDependencyModuleNames.AddRange(new[] { "XmlParser", "Core", "ImageWrapper", "Engine" });

        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.LinuxArm64 ||
            Target.Platform == UnrealTargetPlatform.Linux)
        {
            string platformString = "";
            string libExtension = "";
            string binExtension = "";

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                platformString = "Windows";
                libExtension = ".lib";
                binExtension = ".dll";
                PublicDefinitions.Add("NTDDI_WIN7SP1");
            }
            else if (Target.Platform == UnrealTargetPlatform.LinuxArm64)
            {
                platformString = Path.Combine("Linux", "arm64");
                libExtension = ".so";
                binExtension = ".so";
            }
            else if (Target.Platform == UnrealTargetPlatform.Linux)
            {
                platformString = Path.Combine("Linux", "x64");
                libExtension = ".so";
                binExtension = ".so";
            }
          

            string mySQLPath = Path.Combine(ThirdPartyPath, "MariaDB");
            string mySQLlibraryPath = Path.Combine(mySQLPath, "libraries", platformString);
            string BinariesDir = ProjectBinariesPath;
            
            string pluginpath = Path.Combine(mySQLlibraryPath, "plugin");

            PublicIncludePaths.Add(Path.Combine(mySQLPath, "include"));
            PublicSystemLibraryPaths.Add(mySQLlibraryPath);

            foreach (string file in Directory.GetFiles(mySQLlibraryPath, "*" + libExtension))
            {
               PublicAdditionalLibraries.Add(file);
            }
            
            foreach (string file in Directory.GetFiles(mySQLlibraryPath, "*" + binExtension))
            {
                CopyToBinaries(file, BinariesDir, Target);
            }
            
            foreach (string file in Directory.GetFiles(pluginpath, "*" + libExtension))
            {
                PublicAdditionalLibraries.Add(file);
            }

            foreach (string file in Directory.GetFiles(pluginpath, "*" + binExtension))
            {
                CopyToBinaries(file, BinariesDir, Target);
            }

        }

    }
}