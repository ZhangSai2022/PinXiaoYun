// Orbit camera pawn for inspecting a static mesh or a group of mesh actors.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ModelViewerPawn.generated.h"

class UCameraComponent;
class AModelViewerFocusPointActor;
class UModelViewerFocusTargetComponent;
class UPrimitiveComponent;
class USceneComponent;
class USpringArmComponent;
class UInputComponent;

UCLASS(Blueprintable, BlueprintType)
class DEMO2_API AModelViewerPawn : public APawn
{
	GENERATED_BODY()

public:
	AModelViewerPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Viewer")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Viewer")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Model Viewer")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Defaults", meta = (ClampMin = "1.0"))
	float DefaultZoomDistance = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Defaults")
	float DefaultYaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Defaults")
	float DefaultPitch = -52.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Zoom", meta = (ClampMin = "1.0"))
	float MinZoomDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Zoom", meta = (ClampMin = "1.0"))
	float MaxZoomDistance = 2800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Zoom", meta = (ClampMin = "1.0"))
	float ZoomStep = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Orbit", meta = (ClampMin = "0.001"))
	float OrbitDragSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Orbit", meta = (ClampMin = "0.0"))
	float KeyboardYawSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Orbit", meta = (ClampMin = "0.0"))
	float KeyboardPitchSpeed = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Orbit")
	float MinPitch = -78.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Orbit")
	float MaxPitch = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Pan")
	float PanDragSensitivity = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Smoothing", meta = (ClampMin = "0.0"))
	float RotationInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Smoothing", meta = (ClampMin = "0.0"))
	float ZoomInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Smoothing", meta = (ClampMin = "0.0"))
	float PivotInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Focus", meta = (ClampMin = "1.0"))
	float FocusPadding = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Auto Rotate")
	bool bAutoRotate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Auto Rotate", meta = (EditCondition = "bAutoRotate"))
	float AutoRotateSpeed = 10.0f;

	UFUNCTION(BlueprintCallable, Category = "Model Viewer")
	void ResetView();

	UFUNCTION(BlueprintCallable, Category = "Model Viewer")
	void SetAutoRotate(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void AddZoomInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void ZoomInOneStep();

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void ZoomOutOneStep();

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void AddOrbitInput(FVector2D Delta);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void AddPanInput(FVector2D Delta);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void SetOrbitDragging(bool bDragging);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void SetPanDragging(bool bDragging);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void SetViewAngles(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void SetZoomDistance(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Control")
	void SetPivotLocation(FVector Location);

	// Smoothly frames an arbitrary model actor or one of its mesh components.
	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	void FocusOnActor(AActor* TargetActor, bool bKeepCurrentYaw);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	void FocusOnPrimitive(UPrimitiveComponent* TargetComponent, bool bKeepCurrentYaw);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	void FocusOnLocation(const FVector& Location, float Radius, bool bKeepCurrentYaw);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	void FocusOnTarget(UModelViewerFocusTargetComponent* Target, bool bKeepCurrentYaw);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	void FocusOnFocusPointActor(AModelViewerFocusPointActor* FocusPoint, bool bKeepCurrentYaw);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Focus")
	void FocusOnTargetById(AActor* ModelActor, FName TargetId, bool bKeepCurrentYaw);

protected:
	virtual void BeginPlay() override;

private:
	void Zoom(float Value);
	void KeyboardTurn(float Value);
	void KeyboardLookUp(float Value);
	void OrbitHorizontal(float Value);
	void OrbitVertical(float Value);
	void PanHorizontal(float Value);
	void PanVertical(float Value);
	void BeginOrbitDrag();
	void EndOrbitDrag();
	void BeginPanDrag();
	void EndPanDrag();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Model Viewer|State", meta = (AllowPrivateAccess = "true"))
	float DesiredZoomDistance = 1400.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Model Viewer|State", meta = (AllowPrivateAccess = "true"))
	float DesiredYaw = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Model Viewer|State", meta = (AllowPrivateAccess = "true"))
	float DesiredPitch = -52.0f;
	float InitialZoomDistance = 1400.0f;
	float InitialYaw = 0.0f;
	float InitialPitch = -52.0f;
	FVector InitialPivotLocation = FVector::ZeroVector;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Model Viewer|State", meta = (AllowPrivateAccess = "true"))
	FVector DesiredPivotLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Model Viewer|State", meta = (AllowPrivateAccess = "true"))
	bool bOrbitDragging = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Model Viewer|State", meta = (AllowPrivateAccess = "true"))
	bool bPanDragging = false;
};
