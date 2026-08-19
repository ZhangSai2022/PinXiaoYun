#include "ModelViewerFocusPointActor.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ModelViewerPawn.h"

AModelViewerFocusPointActor::AModelViewerFocusPointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("FocusPointRoot"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FocusSpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = 1400.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FocusCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->bAutoActivate = false;
	Camera->ProjectionMode = ECameraProjectionMode::Perspective;
	Camera->FieldOfView = 90.0f;
	Camera->AspectRatio = 1.777778f;
}

FVector AModelViewerFocusPointActor::GetFocusLocation() const
{
	return GetActorLocation();
}

FRotator AModelViewerFocusPointActor::GetFocusRotation() const
{
	const FRotator Rotation = GetActorRotation();
	return FRotator(Rotation.Pitch, Rotation.Yaw, 0.0f);
}

float AModelViewerFocusPointActor::GetFocusDistance() const
{
	return IsValid(SpringArm) ? FMath::Max(SpringArm->TargetArmLength, 1.0f) : 1.0f;
}

float AModelViewerFocusPointActor::GetFocusFOV() const
{
	return IsValid(Camera) ? Camera->FieldOfView : 90.0f;
}

bool AModelViewerFocusPointActor::FocusViewer(
	AModelViewerPawn* ViewerPawn, bool bKeepCurrentYaw)
{
	if (!IsValid(ViewerPawn))
	{
		return false;
	}

	ViewerPawn->FocusOnFocusPointActor(this, bKeepCurrentYaw);
	return true;
}
