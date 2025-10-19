using UnrealBuildTool;

public class WebSockeProjectTarget : TargetRules
{
	public WebSockeProjectTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange(["WebSockeProject"]);
	}
}
