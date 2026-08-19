// Bird's-eye camera pawn with smoothed spring-arm controls and Blueprint transition events.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlyingPawn.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputComponent;

UCLASS(Blueprintable, BlueprintType)
class DEMO2_API AFlyingPawn : public APawn
{
	GENERATED_BODY()

public:
	AFlyingPawn();

	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Zoom")
	float MinZoomDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Zoom")
	float MaxZoomDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Zoom")
	float ZoomStep = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Smoothing", meta = (ClampMin = "0.0"))
	float ZoomInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Smoothing", meta = (ClampMin = "0.0"))
	float ZoomCompletionTolerance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Rotation")
	float YawSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Rotation")
	float PitchSpeed = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Rotation")
	float MinPitch = -80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Rotation")
	float MaxPitch = -20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bird's Eye Camera|Smoothing", meta = (ClampMin = "0.0"))
	float RotationInterpSpeed = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bird's Eye Camera")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bird's Eye Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bird's Eye Camera")
	TObjectPtr<UCameraComponent> Camera;

	UFUNCTION(BlueprintCallable, Category = "Bird's Eye Camera|Control")
	void Zoom(float Value);

	UFUNCTION(BlueprintCallable, Category = "Bird's Eye Camera|Control")
	void AddZoomInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Bird's Eye Camera|Control")
	void SetZoomDistance(float Distance);

	UFUNCTION(BlueprintPure, Category = "Bird's Eye Camera|Control")
	float GetZoomDistance() const { return DesiredZoomDistance; }

	UFUNCTION(BlueprintCallable, Category = "Bird's Eye Camera|Control")
	void ZoomInOneStep();

	UFUNCTION(BlueprintCallable, Category = "Bird's Eye Camera|Control")
	void ZoomOutOneStep();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bird's Eye Camera|Zoom|Events")
	void OnZoomTransitionStarted(float StartDistance, float TargetDistance);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bird's Eye Camera|Zoom|Events")
	void OnZoomTransitionUpdated(float CurrentDistance, float TargetDistance, float Progress);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bird's Eye Camera|Zoom|Events")
	void OnZoomTransitionFinished(float FinalDistance);

	UFUNCTION(BlueprintCallable, Category = "Bird's Eye Camera|Control")
	void Turn(float Value);

	UFUNCTION(BlueprintCallable, Category = "Bird's Eye Camera|Control")
	void LookUp(float Value);

protected:
	virtual void BeginPlay() override;

private:
	void ConfigureLocalInputMode();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Bird's Eye Camera|State", meta = (AllowPrivateAccess = "true"))
	float DesiredZoomDistance = 1500.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Bird's Eye Camera|State", meta = (AllowPrivateAccess = "true"))
	float ZoomTransitionStartDistance = 1500.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Bird's Eye Camera|State", meta = (AllowPrivateAccess = "true"))
	bool bZoomTransitionActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Bird's Eye Camera|State", meta = (AllowPrivateAccess = "true"))
	float DesiredYaw = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Bird's Eye Camera|State", meta = (AllowPrivateAccess = "true"))
	float DesiredPitch = -55.0f;
};
