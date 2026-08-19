using UnrealBuildTool;
using System.Collections.Generic;

public class DEMO2EditorTarget : TargetRules
{
	public DEMO2EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		ExtraModuleNames.AddRange(new string[] { "DEMO2", "DEMO2Editor" });
	}
}
