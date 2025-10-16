using UnrealBuildTool;

public class WebSockeProjectServerTarget : TargetRules
{
    public WebSockeProjectServerTarget(TargetInfo target) : base(target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
        ExtraModuleNames.Add("WebSockeProject");
    }
}
