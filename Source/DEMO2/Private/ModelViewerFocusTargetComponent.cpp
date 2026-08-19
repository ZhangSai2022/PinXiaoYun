#include "ModelViewerFocusTargetComponent.h"

UModelViewerFocusTargetComponent::UModelViewerFocusTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector UModelViewerFocusTargetComponent::GetFocusLocation() const
{
	return GetComponentTransform().TransformPosition(FocusOffset);
}

FRotator UModelViewerFocusTargetComponent::GetFocusRotation() const
{
	const FRotator MarkerRotation = GetComponentRotation();
	return FRotator(FocusPitch, MarkerRotation.Yaw + FocusYawOffset, 0.0f);
}

float UModelViewerFocusTargetComponent::GetPreviewDistance() const
{
	if (bUseCustomFocusDistance)
	{
		return FMath::Max(FocusDistance, 1.0f);
	}

	const float HalfFovRadians = FMath::DegreesToRadians(PreviewFOV * 0.5f);
	return FMath::Max(FocusRadius / FMath::Tan(HalfFovRadians) * 1.25f, 1.0f);
}
