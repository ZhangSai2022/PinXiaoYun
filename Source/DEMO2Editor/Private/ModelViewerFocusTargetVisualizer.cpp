#include "ModelViewerFocusTargetVisualizer.h"

#include "ModelViewerFocusTargetComponent.h"
#include "SceneManagement.h"

void FModelViewerFocusTargetVisualizer::DrawVisualization(
	const UActorComponent* Component,
	const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	const UModelViewerFocusTargetComponent* Target =
		Cast<const UModelViewerFocusTargetComponent>(Component);
	if (!Target || !PDI)
	{
		return;
	}

	const FVector FocusLocation = Target->GetFocusLocation();
	const FRotator FocusRotation = Target->GetFocusRotation();
	const FVector Forward = FRotationMatrix(FocusRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(FocusRotation).GetUnitAxis(EAxis::Y);
	const FVector Up = FRotationMatrix(FocusRotation).GetUnitAxis(EAxis::Z);
	const float Distance = Target->GetPreviewDistance();
	const FVector CameraLocation = FocusLocation - Forward * Distance;

	const FLinearColor FocusColor(0.05f, 0.85f, 1.0f, 1.0f);
	const FLinearColor CameraColor(1.0f, 0.55f, 0.1f, 1.0f);
	const float LineThickness = 2.0f;

	DrawWireSphere(PDI, FocusLocation, FocusColor,
		FMath::Max(Target->FocusRadius, 2.0f), 16, SDPG_World, LineThickness);
	PDI->DrawLine(FocusLocation, CameraLocation, CameraColor, SDPG_World, LineThickness);
	DrawWireSphere(PDI, CameraLocation, CameraColor, 10.0f, 12, SDPG_World, LineThickness);

	const float HalfVertical = FMath::Tan(FMath::DegreesToRadians(Target->PreviewFOV * 0.5f)) * Distance;
	const float HalfHorizontal = HalfVertical * FMath::Max(Target->PreviewAspectRatio, 0.1f);
	const FVector NearCenter = CameraLocation + Forward * 20.0f;
	const FVector FarCenter = FocusLocation;
	const float NearHalfVertical = HalfVertical * 0.08f;
	const float NearHalfHorizontal = HalfHorizontal * 0.08f;

	const FVector NearCorners[4] =
	{
		NearCenter + Right * NearHalfHorizontal + Up * NearHalfVertical,
		NearCenter - Right * NearHalfHorizontal + Up * NearHalfVertical,
		NearCenter - Right * NearHalfHorizontal - Up * NearHalfVertical,
		NearCenter + Right * NearHalfHorizontal - Up * NearHalfVertical
	};
	const FVector FarCorners[4] =
	{
		FarCenter + Right * HalfHorizontal + Up * HalfVertical,
		FarCenter - Right * HalfHorizontal + Up * HalfVertical,
		FarCenter - Right * HalfHorizontal - Up * HalfVertical,
		FarCenter + Right * HalfHorizontal - Up * HalfVertical
	};

	for (int32 Index = 0; Index < 4; ++Index)
	{
		const int32 Next = (Index + 1) % 4;
		PDI->DrawLine(NearCorners[Index], NearCorners[Next], CameraColor, SDPG_World, LineThickness);
		PDI->DrawLine(FarCorners[Index], FarCorners[Next], FocusColor, SDPG_World, LineThickness);
		PDI->DrawLine(NearCorners[Index], FarCorners[Index], CameraColor, SDPG_World, LineThickness);
	}

	PDI->DrawLine(FocusLocation, FocusLocation + Forward * 100.0f, FocusColor, SDPG_World, LineThickness);
}
