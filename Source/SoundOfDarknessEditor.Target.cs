using UnrealBuildTool;
using System.Collections.Generic;

public class SoundOfDarknessEditorTarget : TargetRules
{
	public SoundOfDarknessEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("SoundOfDarkness");
	}
}
