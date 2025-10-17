using UnrealBuildTool;
using System.IO;

public class MQTTPlugin : ModuleRules
{
    public MQTTPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        // --- ThirdParty setup ---
        string ThirdPartyPath = Path.Combine(ModuleDirectory, "../../ThirdParty/paho");
        string IncludePath = Path.Combine(ThirdPartyPath, "include");
        string LibPath = Path.Combine(ThirdPartyPath, "lib", Target.Platform.ToString());

        PublicIncludePaths.Add(IncludePath);

        // --- Platform specific linking ---
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string LibName = "paho-mqtt3a.lib";
            string FullLibPath = Path.Combine(LibPath, LibName);
            PublicAdditionalLibraries.Add(FullLibPath);

            // Optional: load DLL at runtime if not statically linked
            string DllPath = Path.Combine(LibPath, "paho-mqtt3a.dll");
            if (File.Exists(DllPath))
            {
                RuntimeDependencies.Add("$(BinaryOutputDir)/paho-mqtt3a.dll", DllPath);
                PublicDelayLoadDLLs.Add("paho-mqtt3a.dll");
            }

            PublicDefinitions.Add("WITH_MQTT_WINDOWS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            // Link against shared library .so
            string LibName = "libpaho-mqtt3a.so";
            string FullLibPath = Path.Combine(LibPath, LibName);
            PublicAdditionalLibraries.Add(FullLibPath);

            RuntimeDependencies.Add("$(BinaryOutputDir)/libpaho-mqtt3a.so", FullLibPath);
            PublicDefinitions.Add("WITH_MQTT_LINUX=1");
        }

        bEnableExceptions = true;
        bUseRTTI = true;
    }
}
