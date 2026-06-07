using UnrealBuildTool;

public class SoundOfDarkness : ModuleRules
{
	public SoundOfDarkness(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "HeadMountedDisplay", "AIModule", "GameplayTasks", "NavigationSystem", "UMG", "Slate", "SlateCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "MetasoundEngine", "AudioCapture", "AudioMixer" });
	}
}
