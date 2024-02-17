// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Testue5 : ModuleRules
{
	public Testue5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "NavigationSystem", "AIModule" });
	}
}
