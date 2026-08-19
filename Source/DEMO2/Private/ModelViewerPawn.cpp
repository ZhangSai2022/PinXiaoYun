#include "ModelViewerPawn.h"

#include "ModelViewerFocusPointActor.h"
#include "ModelViewerFocusTargetComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"

namespace
{
FRotator ToParentRelativeRotation(
	const USceneComponent* Parent, const FRotator& WorldRotation)
{
	return (Parent->GetComponentQuat().Inverse() * WorldRotation.Quaternion()).Rotator();
}
}

AModelViewerPawn::AModelViewerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("ModelViewerRoot"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ModelViewerSpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = DefaultZoomDistance;
	SpringArm->SetRelativeRotation(FRotator(DefaultPitch, DefaultYaw, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 14.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 14.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ModelViewerCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->ProjectionMode = ECameraProjectionMode::Perspective;
	Camera->FieldOfView = 90.0f;
	Camera->AspectRatio = 1.777778f;
}

void AModelViewerPawn::BeginPlay()
{
	Super::BeginPlay();

	DesiredZoomDistance = FMath::Clamp(DefaultZoomDistance, MinZoomDistance, MaxZoomDistance);
	SpringArm->TargetArmLength = DesiredZoomDistance;
	// Blueprint component defaults are applied after the C++ constructor.
	// Use the authored SpringArm angle instead of overwriting it at runtime.
	const FRotator InitialRotation = SpringArm->GetRelativeRotation();
	DesiredYaw = InitialRotation.Yaw;
	DesiredPitch = FMath::Clamp(InitialRotation.Pitch, MinPitch, MaxPitch);
	SpringArm->SetRelativeRotation(FRotator(DesiredPitch, DesiredYaw, 0.0f));
	InitialZoomDistance = DesiredZoomDistance;
	InitialYaw = DesiredYaw;
	InitialPitch = DesiredPitch;
	InitialPivotLocation = GetActorLocation();
	DesiredPivotLocation = InitialPivotLocation;

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
	}
}

void AModelViewerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Delta = FMath::Clamp(DeltaSeconds, 0.0f, 0.1f);
	SetActorLocation(FMath::VInterpTo(
		GetActorLocation(), DesiredPivotLocation, Delta, PivotInterpSpeed));

	if (bAutoRotate && !bOrbitDragging)
	{
		DesiredYaw += AutoRotateSpeed * Delta;
	}

	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength, DesiredZoomDistance, Delta, ZoomInterpSpeed);

	const FRotator CurrentRotation = SpringArm->GetRelativeRotation();
	const float YawDelta = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredYaw);
	const float YawAlpha = 1.0f - FMath::Exp(-RotationInterpSpeed * Delta);
	const float NewYaw = CurrentRotation.Yaw + YawDelta * YawAlpha;
	const float NewPitch = FMath::FInterpTo(
		CurrentRotation.Pitch, DesiredPitch, Delta, RotationInterpSpeed);
	SpringArm->SetRelativeRotation(FRotator(NewPitch, NewYaw, 0.0f));
}

void AModelViewerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("ModelViewerZoom"), this, &AModelViewerPawn::Zoom);
	PlayerInputComponent->BindAxis(TEXT("ModelViewerTurn"), this, &AModelViewerPawn::KeyboardTurn);
	PlayerInputComponent->BindAxis(TEXT("ModelViewerLookUp"), this, &AModelViewerPawn::KeyboardLookUp);
	PlayerInputComponent->BindAxis(TEXT("ModelViewerOrbitX"), this, &AModelViewerPawn::OrbitHorizontal);
	PlayerInputComponent->BindAxis(TEXT("ModelViewerOrbitY"), this, &AModelViewerPawn::OrbitVertical);
	PlayerInputComponent->BindAxis(TEXT("ModelViewerPanX"), this, &AModelViewerPawn::PanHorizontal);
	PlayerInputComponent->BindAxis(TEXT("ModelViewerPanY"), this, &AModelViewerPawn::PanVertical);

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AModelViewerPawn::BeginOrbitDrag);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AModelViewerPawn::EndOrbitDrag);
	PlayerInputComponent->BindKey(EKeys::MiddleMouseButton, IE_Pressed, this, &AModelViewerPawn::BeginPanDrag);
	PlayerInputComponent->BindKey(EKeys::MiddleMouseButton, IE_Released, this, &AModelViewerPawn::EndPanDrag);
	PlayerInputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AModelViewerPawn::ZoomInOneStep);
	PlayerInputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AModelViewerPawn::ZoomOutOneStep);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &AModelViewerPawn::ResetView);
}

void AModelViewerPawn::Zoom(float Value)
{
	AddZoomInput(Value);
}

void AModelViewerPawn::KeyboardTurn(float Value)
{
	DesiredYaw += Value * KeyboardYawSpeed * GetWorld()->GetDeltaSeconds();
}

void AModelViewerPawn::KeyboardLookUp(float Value)
{
	DesiredPitch = FMath::Clamp(
		DesiredPitch + Value * KeyboardPitchSpeed * GetWorld()->GetDeltaSeconds(), MinPitch, MaxPitch);
}

void AModelViewerPawn::OrbitHorizontal(float Value)
{
	if (bOrbitDragging)
	{
		AddOrbitInput(FVector2D(Value, 0.0f));
	}
}

void AModelViewerPawn::OrbitVertical(float Value)
{
	if (bOrbitDragging)
	{
		AddOrbitInput(FVector2D(0.0f, Value));
	}
}

void AModelViewerPawn::PanHorizontal(float Value)
{
	if (bPanDragging)
	{
		AddPanInput(FVector2D(Value, 0.0f));
	}
}

void AModelViewerPawn::PanVertical(float Value)
{
	if (bPanDragging)
	{
		AddPanInput(FVector2D(0.0f, Value));
	}
}

void AModelViewerPawn::BeginOrbitDrag() { SetOrbitDragging(true); }
void AModelViewerPawn::EndOrbitDrag() { SetOrbitDragging(false); }
void AModelViewerPawn::BeginPanDrag() { SetPanDragging(true); }
void AModelViewerPawn::EndPanDrag() { SetPanDragging(false); }

void AModelViewerPawn::ResetView()
{
	DesiredZoomDistance = InitialZoomDistance;
	DesiredYaw = InitialYaw;
	DesiredPitch = InitialPitch;
	DesiredPivotLocation = InitialPivotLocation;
}

void AModelViewerPawn::SetAutoRotate(bool bEnabled)
{
	bAutoRotate = bEnabled;
}

void AModelViewerPawn::AddZoomInput(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		SetZoomDistance(DesiredZoomDistance - Value * ZoomStep);
	}
}

void AModelViewerPawn::ZoomInOneStep()
{
	AddZoomInput(1.0f);
}

void AModelViewerPawn::ZoomOutOneStep()
{
	AddZoomInput(-1.0f);
}

void AModelViewerPawn::AddOrbitInput(FVector2D Delta)
{
	DesiredYaw += Delta.X * OrbitDragSensitivity;
	DesiredPitch = FMath::Clamp(
		DesiredPitch + Delta.Y * OrbitDragSensitivity, MinPitch, MaxPitch);
}

void AModelViewerPawn::AddPanInput(FVector2D Delta)
{
	const FRotator YawRotation(0.0f, DesiredYaw, 0.0f);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	DesiredPivotLocation += (-Right * Delta.X - Forward * Delta.Y) * PanDragSensitivity;
}

void AModelViewerPawn::SetOrbitDragging(bool bDragging)
{
	bOrbitDragging = bDragging;
}

void AModelViewerPawn::SetPanDragging(bool bDragging)
{
	bPanDragging = bDragging;
}

void AModelViewerPawn::SetViewAngles(float Yaw, float Pitch)
{
	DesiredYaw = Yaw;
	DesiredPitch = FMath::Clamp(Pitch, MinPitch, MaxPitch);
}

void AModelViewerPawn::SetZoomDistance(float Distance)
{
	DesiredZoomDistance = FMath::Clamp(Distance, MinZoomDistance, MaxZoomDistance);
}

void AModelViewerPawn::SetPivotLocation(FVector Location)
{
	DesiredPivotLocation = Location;
}

void AModelViewerPawn::FocusOnActor(AActor* TargetActor, bool bKeepCurrentYaw)
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	const FBox Bounds = TargetActor->GetComponentsBoundingBox(true);
	if (Bounds.IsValid)
	{
		FocusOnLocation(Bounds.GetCenter(), Bounds.GetExtent().Size(), bKeepCurrentYaw);
	}
	else
	{
		FocusOnLocation(TargetActor->GetActorLocation(), 100.0f, bKeepCurrentYaw);
	}
}

void AModelViewerPawn::FocusOnPrimitive(UPrimitiveComponent* TargetComponent, bool bKeepCurrentYaw)
{
	if (!IsValid(TargetComponent))
	{
		return;
	}

	const FBoxSphereBounds Bounds = TargetComponent->CalcBounds(TargetComponent->GetComponentTransform());
	FocusOnLocation(Bounds.Origin, FMath::Max(Bounds.SphereRadius, 1.0f), bKeepCurrentYaw);
}

void AModelViewerPawn::FocusOnLocation(const FVector& Location, float Radius, bool bKeepCurrentYaw)
{
	DesiredPivotLocation = Location;
	const float SafeRadius = FMath::Max(Radius, 1.0f);
	const float HalfFovRadians = FMath::DegreesToRadians(Camera->FieldOfView * 0.5f);
	const float FitDistance = SafeRadius / FMath::Tan(HalfFovRadians);
	DesiredZoomDistance = FMath::Clamp(FitDistance * FocusPadding, MinZoomDistance, MaxZoomDistance);

	if (!bKeepCurrentYaw)
	{
		DesiredYaw = 0.0f;
	}
}

void AModelViewerPawn::FocusOnTarget(UModelViewerFocusTargetComponent* Target, bool bKeepCurrentYaw)
{
	if (!IsValid(Target))
	{
		return;
	}

	FocusOnLocation(Target->GetFocusLocation(), Target->FocusRadius, bKeepCurrentYaw);
	if (Target->bUseCustomFocusDistance)
	{
		SetZoomDistance(Target->FocusDistance);
	}

	if (Target->bUseCustomViewAngles)
	{
		const FRotator ViewRotation = ToParentRelativeRotation(
			SpringArm->GetAttachParent(), Target->GetFocusRotation());
		SetViewAngles(ViewRotation.Yaw, ViewRotation.Pitch);
	}
}

void AModelViewerPawn::FocusOnFocusPointActor(
	AModelViewerFocusPointActor* FocusPoint, bool bKeepCurrentYaw)
{
	if (!IsValid(FocusPoint))
	{
		return;
	}

	SetPivotLocation(FocusPoint->GetFocusLocation());
	SetZoomDistance(FocusPoint->GetFocusDistance());
	if (!bKeepCurrentYaw)
	{
		const FRotator ViewRotation = ToParentRelativeRotation(
			SpringArm->GetAttachParent(), FocusPoint->GetFocusRotation());
		SetViewAngles(ViewRotation.Yaw, ViewRotation.Pitch);
	}

	if (FocusPoint->bApplyCameraFOV && IsValid(Camera))
	{
		Camera->SetFieldOfView(FocusPoint->GetFocusFOV());
	}
}

void AModelViewerPawn::FocusOnTargetById(AActor* ModelActor, FName TargetId, bool bKeepCurrentYaw)
{
	if (!IsValid(ModelActor) || TargetId.IsNone())
	{
		return;
	}

	TArray<UModelViewerFocusTargetComponent*> Targets;
	ModelActor->GetComponents<UModelViewerFocusTargetComponent>(Targets);
	for (UModelViewerFocusTargetComponent* Target : Targets)
	{
		if (IsValid(Target) && Target->TargetId == TargetId)
		{
			FocusOnTarget(Target, bKeepCurrentYaw);
			return;
		}
	}

	TArray<AActor*> AttachedActors;
	ModelActor->GetAttachedActors(AttachedActors, true, true);
	for (AActor* AttachedActor : AttachedActors)
	{
		AModelViewerFocusPointActor* FocusPoint = Cast<AModelViewerFocusPointActor>(AttachedActor);
		if (IsValid(FocusPoint) && FocusPoint->TargetId == TargetId)
		{
			FocusOnFocusPointActor(FocusPoint, bKeepCurrentYaw);
			return;
		}
	}
}
