using UnrealBuildTool;
using System.IO;

public class MQTTPlugin : ModuleRules
{
    public MQTTPlugin(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(["Core",
            "CoreUObject",
            "Engine"]);

        // --- ThirdParty setup ---
        var thirdPartyPath = Path.Combine(ModuleDirectory, "../../ThirdParty/paho");
        var includePath = Path.Combine(thirdPartyPath, "include");
        var libPath = Path.Combine(thirdPartyPath, "lib", Target.Platform.ToString());

        PublicIncludePaths.Add(includePath);

        // --- Default Unreal behavior ---
        bEnableExceptions = true;
        bUseRTTI = false; // ❗ Always false for UObject-based modules

        // --- Platform specific linking ---
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            const string libName = "paho-mqtt3a.lib";
            var fullLibPath = Path.Combine(libPath, libName);
            PublicAdditionalLibraries.Add(fullLibPath);

            var dllPath = Path.Combine(libPath, "paho-mqtt3a.dll");
            if (File.Exists(dllPath))
            {
                RuntimeDependencies.Add("$(BinaryOutputDir)/paho-mqtt3a.dll", dllPath);
                PublicDelayLoadDLLs.Add("paho-mqtt3a.dll");
            }

            PublicDefinitions.Add("WITH_MQTT_WINDOWS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            var libraryPath = Path.Combine(ModuleDirectory, "../../ThirdParty/paho/lib/Linux");

            PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "libpaho-mqtt3a.so"));

            // Copy all paho libraries to output
            foreach (var soFile in Directory.GetFiles(libraryPath, "libpaho-*.so*"))
            {
                var fileName = Path.GetFileName(soFile);
                RuntimeDependencies.Add("$(TargetOutputDir)/" + fileName, soFile);
            }

            PublicDefinitions.Add("WITH_MQTT_LINUX=1");
        }
    }
}
