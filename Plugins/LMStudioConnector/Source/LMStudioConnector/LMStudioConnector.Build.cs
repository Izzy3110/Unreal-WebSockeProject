using UnrealBuildTool;

public class LMStudioConnector : ModuleRules
{
	public LMStudioConnector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			[
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore"
			]
		);

		PrivateDependencyModuleNames.AddRange(
			[
				"Projects",
				"Settings",
				"Slate",
				"SlateCore",
				"HTTP"
			]
		);
	}
}