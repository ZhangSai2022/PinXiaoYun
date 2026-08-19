// A transform marker used by model-viewer menus to focus a sub-assembly.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ModelViewerFocusTargetComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (ModelViewer), meta = (BlueprintSpawnableComponent))
class DEMO2_API UModelViewerFocusTargetComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UModelViewerFocusTargetComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus")
	FName TargetId;

	// Offset from this marker, expressed in the marker's local space.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus")
	FVector FocusOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus", meta = (ClampMin = "1.0"))
	float FocusRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus")
	bool bUseCustomFocusDistance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus", meta = (EditCondition = "bUseCustomFocusDistance", ClampMin = "1.0"))
	float FocusDistance = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Editor Preview", meta = (ClampMin = "1.0", ClampMax = "179.0"))
	float PreviewFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Editor Preview", meta = (ClampMin = "0.1"))
	float PreviewAspectRatio = 1.777778f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus")
	bool bUseCustomViewAngles = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus", meta = (EditCondition = "bUseCustomViewAngles"))
	float FocusYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus", meta = (EditCondition = "bUseCustomViewAngles"))
	float FocusPitch = -52.0f;

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Focus")
	FVector GetFocusLocation() const;

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Focus")
	FRotator GetFocusRotation() const;

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Focus")
	float GetPreviewDistance() const;
};
