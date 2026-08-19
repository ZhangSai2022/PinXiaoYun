#include "ModelViewerFocusPointChildComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ModelViewerFocusPointActor.h"
#include "ModelViewerPawn.h"

UModelViewerFocusPointChildComponent::UModelViewerFocusPointChildComponent()
{
	SetChildActorClass(AModelViewerFocusPointActor::StaticClass());
	SetChildActorOwnerOnCreation(true);
#if WITH_EDITOR
	SetEditorTreeViewVisualizationMode(
		EChildActorComponentTreeViewVisualizationMode::ComponentWithChildActor);
#endif
}

AModelViewerFocusPointActor* UModelViewerFocusPointChildComponent::GetFocusPointActor() const
{
	return Cast<AModelViewerFocusPointActor>(GetChildActor());
}

bool UModelViewerFocusPointChildComponent::FocusViewer(
	AModelViewerPawn* ViewerPawn, bool bKeepCurrentYaw)
{
	AModelViewerFocusPointActor* FocusPoint = GetFocusPointActor();
	if (!IsValid(ViewerPawn) || !IsValid(FocusPoint))
	{
		return false;
	}

	ViewerPawn->FocusOnFocusPointActor(FocusPoint, bKeepCurrentYaw);
	return true;
}

void UModelViewerFocusPointChildComponent::RefreshFocusPoint()
{
	ApplySettingsToFocusPoint();
}

void UModelViewerFocusPointChildComponent::OnRegister()
{
	Super::OnRegister();
	ApplySettingsToFocusPoint();
}

void UModelViewerFocusPointChildComponent::CreateChildActor(
	TFunction<void(AActor*)> CustomizerFunc)
{
	Super::CreateChildActor(MoveTemp(CustomizerFunc));
	ApplySettingsToFocusPoint();
}

#if WITH_EDITOR
void UModelViewerFocusPointChildComponent::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplySettingsToFocusPoint();
}

void UModelViewerFocusPointChildComponent::PostEditUndo()
{
	Super::PostEditUndo();
	ApplySettingsToFocusPoint();
}
#endif

void UModelViewerFocusPointChildComponent::ApplySettingsToFocusPoint()
{
	AModelViewerFocusPointActor* FocusPoint = GetFocusPointActor();
	if (!IsValid(FocusPoint))
	{
		return;
	}

	FocusPoint->TargetId = TargetId;
	FocusPoint->bApplyCameraFOV = bApplyCameraFOV;
	if (IsValid(FocusPoint->SpringArm))
	{
		FocusPoint->SpringArm->TargetArmLength = FMath::Max(FocusDistance, 1.0f);
	}
	if (IsValid(FocusPoint->Camera))
	{
		FocusPoint->Camera->SetFieldOfView(FMath::Clamp(FocusFOV, 1.0f, 179.0f));
	}
}
