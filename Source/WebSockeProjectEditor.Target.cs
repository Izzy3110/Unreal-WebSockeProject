// Fill out your copyright notice in the Description page of Project Settings.
using UnrealBuildTool;

public class WebSockeProjectEditorTarget : TargetRules
{
	public WebSockeProjectEditorTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		
		ExtraModuleNames.AddRange( new string[] { "WebSockeProject" } );
	}
}
