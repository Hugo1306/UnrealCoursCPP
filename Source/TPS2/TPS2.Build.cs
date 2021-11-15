// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TPS2 : ModuleRules
{
	public TPS2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
