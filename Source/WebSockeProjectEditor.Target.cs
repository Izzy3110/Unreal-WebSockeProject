using UnrealBuildTool;

public class WebSockeProjectEditorTarget : TargetRules
{
	public WebSockeProjectEditorTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		
		ExtraModuleNames.AddRange( ["WebSockeProject"]);
		bAllowHotReload = false;
	}
}
