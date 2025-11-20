using System.IO;
using UnrealBuildTool;

public class YamlPlugin : ModuleRules
{
	public YamlPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableExceptions = true; // yaml-cpp nutzt Exceptions

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Json", "JsonUtilities" });

		string ModuleDir = ModuleDirectory;
		string PluginDir = Path.Combine(ModuleDir, "..", ".."); // Plugin root
		string ThirdPartyDir = Path.Combine(PluginDir, "ThirdParty");
		string YamlCppDir = Path.Combine(ThirdPartyDir, "yaml-cpp");

		// Header-Pfad für yaml-cpp
		PublicIncludePaths.Add(Path.Combine(YamlCppDir, "include"));

		// statische Library einbinden
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string LibPath = Path.Combine(YamlCppDir, "lib", "Win64", "yaml-cpp.lib");
			PublicAdditionalLibraries.Add(LibPath);
			PublicDefinitions.Add("YAML_CPP_STATIC_DEFINE");
		}
	}
}
