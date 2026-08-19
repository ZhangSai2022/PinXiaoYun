// A configurable ChildActorComponent that exposes focus-rig settings on its owner Blueprint.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChildActorComponent.h"
#include "ModelViewerFocusPointChildComponent.generated.h"

class AModelViewerFocusPointActor;
class AModelViewerPawn;

UCLASS(Blueprintable, BlueprintType, ClassGroup = (ModelViewer), meta = (BlueprintSpawnableComponent))
class DEMO2_API UModelViewerFocusPointChildComponent : public UChildActorComponent
{
	GENERATED_BODY()

public:
	UModelViewerFocusPointChildComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus")
	FName TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus", meta = (ClampMin = "1.0"))
	float FocusDistance = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus", meta = (ClampMin = "1.0", ClampMax = "179.0"))
	float FocusFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus")
	bool bApplyCameraFOV = true;

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Focus")
	AModelViewerFocusPointActor* GetFocusPointActor() const;

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	bool FocusViewer(AModelViewerPawn* ViewerPawn, bool bKeepCurrentYaw);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	void RefreshFocusPoint();

	virtual void OnRegister() override;
	virtual void CreateChildActor(TFunction<void(AActor*)> CustomizerFunc = nullptr) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
#endif

private:
	void ApplySettingsToFocusPoint();
};
