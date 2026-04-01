using UnrealBuildTool;
using System.Collections.Generic;

public class SoundOfDarknessTarget : TargetRules
{
	public SoundOfDarknessTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("SoundOfDarkness");
	}
}
