using UnrealBuildTool;

public class CommonUtils : ModuleRules
{
	public CommonUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		// Avoid Live Coding “precompiled object not linked” issues
		PCHUsage = PCHUsageMode.NoPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "AssetRegistry" });
	}
}