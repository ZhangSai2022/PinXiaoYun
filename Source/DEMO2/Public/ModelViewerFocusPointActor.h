// A visible, reusable focus preset authored with a spring arm and camera.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModelViewerFocusPointActor.generated.h"

class AModelViewerPawn;
class UCameraComponent;
class USceneComponent;
class USpringArmComponent;

UCLASS(Blueprintable, BlueprintType)
class DEMO2_API AModelViewerFocusPointActor : public AActor
{
	GENERATED_BODY()

public:
	AModelViewerFocusPointActor();

	// The menu/button id used to find this preset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus")
	FName TargetId;

	// Apply the preset camera's FOV to the viewer pawn when focusing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus")
	bool bApplyCameraFOV = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Viewer")
	TObjectPtr<USceneComponent> Root;

	// TargetArmLength is the authored focus distance.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Viewer")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Viewer")
	TObjectPtr<UCameraComponent> Camera;

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Focus")
	FVector GetFocusLocation() const;

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Focus")
	FRotator GetFocusRotation() const;

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Focus")
	float GetFocusDistance() const;

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Focus")
	float GetFocusFOV() const;

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	bool FocusViewer(AModelViewerPawn* ViewerPawn, bool bKeepCurrentYaw);
};
