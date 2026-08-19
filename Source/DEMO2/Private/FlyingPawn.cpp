#include "FlyingPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"

AFlyingPawn::AFlyingPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = DesiredZoomDistance;
	SpringArm->SetRelativeRotation(FRotator(DesiredPitch, DesiredYaw, 0.0f));
	SpringArm->bDoCollisionTest = true;
	SpringArm->ProbeSize = 12.0f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 12.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 12.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void AFlyingPawn::BeginPlay()
{
	Super::BeginPlay();

	DesiredZoomDistance = FMath::Clamp(SpringArm->TargetArmLength, MinZoomDistance, MaxZoomDistance);
	SpringArm->TargetArmLength = DesiredZoomDistance;
	ZoomTransitionStartDistance = DesiredZoomDistance;
	const FRotator InitialRotation = SpringArm->GetRelativeRotation();
	DesiredYaw = InitialRotation.Yaw;
	DesiredPitch = FMath::Clamp(InitialRotation.Pitch, MinPitch, MaxPitch);
	ConfigureLocalInputMode();
}

void AFlyingPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ConfigureLocalInputMode();
}

void AFlyingPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!SpringArm)
	{
		return;
	}

	const float InterpDelta = FMath::Clamp(DeltaTime, 0.0f, 0.1f);
	float NewZoomDistance = FMath::FInterpTo(
		SpringArm->TargetArmLength, DesiredZoomDistance, InterpDelta, ZoomInterpSpeed);

	if (bZoomTransitionActive)
	{
		const bool bTransitionFinished = FMath::IsNearlyEqual(
			NewZoomDistance, DesiredZoomDistance, ZoomCompletionTolerance);
		if (bTransitionFinished)
		{
			NewZoomDistance = DesiredZoomDistance;
		}

		const float TransitionDistance = FMath::Abs(DesiredZoomDistance - ZoomTransitionStartDistance);
		const float RemainingDistance = FMath::Abs(DesiredZoomDistance - NewZoomDistance);
		const float Progress = TransitionDistance <= ZoomCompletionTolerance
			? 1.0f
			: FMath::Clamp(1.0f - RemainingDistance / TransitionDistance, 0.0f, 1.0f);
		SpringArm->TargetArmLength = NewZoomDistance;
		OnZoomTransitionUpdated(NewZoomDistance, DesiredZoomDistance, Progress);

		if (bTransitionFinished)
		{
			bZoomTransitionActive = false;
			OnZoomTransitionFinished(NewZoomDistance);
		}
	}
	else
	{
		SpringArm->TargetArmLength = NewZoomDistance;
	}

	const FRotator CurrentRotation = SpringArm->GetRelativeRotation();
	const float YawDelta = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredYaw);
	const float YawAlpha = 1.0f - FMath::Exp(-RotationInterpSpeed * InterpDelta);
	const float CurrentYaw = CurrentRotation.Yaw + YawDelta * YawAlpha;
	const float CurrentPitch = FMath::FInterpTo(
		CurrentRotation.Pitch, DesiredPitch, InterpDelta, RotationInterpSpeed);
	SpringArm->SetRelativeRotation(FRotator(CurrentPitch, CurrentYaw, 0.0f));

}

void AFlyingPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AFlyingPawn::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AFlyingPawn::LookUp);

	// Bind discrete wheel keys so zoom does not depend on a legacy axis mapping.
	PlayerInputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AFlyingPawn::ZoomInOneStep);
	PlayerInputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AFlyingPawn::ZoomOutOneStep);
}

void AFlyingPawn::Zoom(float Value)
{
	SetZoomDistance(Value);
}

void AFlyingPawn::AddZoomInput(float Value)
{
	const float ClampedInput = FMath::Clamp(Value, -1.0f, 1.0f);
	if (FMath::IsNearlyZero(ClampedInput))
	{
		return;
	}

	SetZoomDistance(DesiredZoomDistance - ClampedInput * ZoomStep);
}

void AFlyingPawn::SetZoomDistance(float Distance)
{
	const float NewDesiredZoomDistance = FMath::Clamp(Distance, MinZoomDistance, MaxZoomDistance);
	if (FMath::IsNearlyEqual(NewDesiredZoomDistance, DesiredZoomDistance))
	{
		return;
	}

	ZoomTransitionStartDistance = SpringArm ? SpringArm->TargetArmLength : DesiredZoomDistance;
	DesiredZoomDistance = NewDesiredZoomDistance;
	bZoomTransitionActive = true;
	OnZoomTransitionStarted(ZoomTransitionStartDistance, DesiredZoomDistance);
}

void AFlyingPawn::ZoomInOneStep()
{
	AddZoomInput(1.0f);
}

void AFlyingPawn::ZoomOutOneStep()
{
	AddZoomInput(-1.0f);
}

void AFlyingPawn::Turn(float Value)
{
	if (GetWorld())
	{
		DesiredYaw += Value * YawSpeed * GetWorld()->GetDeltaSeconds();
	}
}

void AFlyingPawn::LookUp(float Value)
{
	if (GetWorld())
	{
		DesiredPitch = FMath::Clamp(
			DesiredPitch + Value * PitchSpeed * GetWorld()->GetDeltaSeconds(), MinPitch, MaxPitch);
	}
}

void AFlyingPawn::ConfigureLocalInputMode()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	PlayerController->bShowMouseCursor = true;
	PlayerController->bEnableClickEvents = true;
	PlayerController->bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
}
