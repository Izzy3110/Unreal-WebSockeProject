using UnrealBuildTool;

public class RenderWebSocketStreamer : ModuleRules
{
	public RenderWebSocketStreamer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"RenderCore",
			"RHI",
			"ImageWrapper",
			"WebSockets"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Projects"
		});

		// For reading pixels from render target
		bEnableExceptions = false;
		CppStandard = CppStandardVersion.Cpp20;
	}
}
