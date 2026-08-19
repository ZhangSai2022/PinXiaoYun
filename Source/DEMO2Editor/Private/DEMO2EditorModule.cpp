#include "Modules/ModuleManager.h"

#include "Editor/UnrealEdEngine.h"
#include "Misc/CoreDelegates.h"
#include "ModelViewerFocusTargetComponent.h"
#include "ModelViewerFocusTargetVisualizer.h"
#include "UnrealEdGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogDEMO2Editor, Log, All);

class FDEMO2EditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		// The editor engine can be initialized after this module. Register immediately
		// when possible and retry after engine initialization when it is not.
		PostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddRaw(
			this, &FDEMO2EditorModule::RegisterVisualizer);
		RegisterVisualizer();
	}

	virtual void ShutdownModule() override
	{
		if (PostEngineInitHandle.IsValid())
		{
			FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);
			PostEngineInitHandle.Reset();
		}

		if (GUnrealEd && !VisualizerClassName.IsNone())
		{
			GUnrealEd->UnregisterComponentVisualizer(VisualizerClassName);
		}
		Visualizer.Reset();
	}

private:
	void RegisterVisualizer()
	{
		if (!GUnrealEd || Visualizer.IsValid())
		{
			return;
		}

		VisualizerClassName = UModelViewerFocusTargetComponent::StaticClass()->GetFName();
		Visualizer = MakeShared<FModelViewerFocusTargetVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(VisualizerClassName, Visualizer);
		Visualizer->OnRegister();

		UE_LOG(LogDEMO2Editor, Log,
			TEXT("Registered component visualizer for %s"),
			*VisualizerClassName.ToString());
	}

	FName VisualizerClassName;
	TSharedPtr<FModelViewerFocusTargetVisualizer> Visualizer;
	FDelegateHandle PostEngineInitHandle;
};

IMPLEMENT_MODULE(FDEMO2EditorModule, DEMO2Editor)
