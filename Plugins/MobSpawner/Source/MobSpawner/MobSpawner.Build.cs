using UnrealBuildTool;

public class MobSpawner : ModuleRules
{
	public MobSpawner(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange([
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"GameplayTags",
			"RenderCore"
		]);

		PrivateDependencyModuleNames.AddRange([
			"Slate",
			"SlateCore"
		]);
	}
}
