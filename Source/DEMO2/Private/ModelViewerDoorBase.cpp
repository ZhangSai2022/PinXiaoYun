#include "ModelViewerDoorBase.h"

#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "ModelViewerFocusPointActor.h"
#include "ModelViewerFocusTargetComponent.h"
#include "ModelViewerPawn.h"

AModelViewerDoorBase::AModelViewerDoorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	DoorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	SetRootComponent(DoorRoot);

	ModelRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ModelRoot"));
	ModelRoot->SetupAttachment(DoorRoot);

	FocusTargetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FocusTargetRoot"));
	FocusTargetRoot->SetupAttachment(DoorRoot);
}

void AModelViewerDoorBase::BeginPlay()
{
	Super::BeginPlay();
}

UModelViewerFocusTargetComponent* AModelViewerDoorBase::FindFocusTarget(FName TargetId) const
{
	if (TargetId.IsNone())
	{
		return nullptr;
	}

	TArray<UModelViewerFocusTargetComponent*> Targets;
	GetComponents<UModelViewerFocusTargetComponent>(Targets);
	for (UModelViewerFocusTargetComponent* Target : Targets)
	{
		if (IsValid(Target) && Target->TargetId == TargetId)
		{
			return Target;
		}
	}

	return nullptr;
}

AModelViewerFocusPointActor* AModelViewerDoorBase::FindFocusPointActor(FName TargetId) const
{
	if (TargetId.IsNone())
	{
		return nullptr;
	}

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors, true, true);
	for (AActor* AttachedActor : AttachedActors)
	{
		AModelViewerFocusPointActor* FocusPoint = Cast<AModelViewerFocusPointActor>(AttachedActor);
		if (IsValid(FocusPoint) && FocusPoint->TargetId == TargetId)
		{
			return FocusPoint;
		}
	}

	return nullptr;
}

TArray<FName> AModelViewerDoorBase::GetFocusTargetIds() const
{
	TArray<FName> Result;
	TArray<UModelViewerFocusTargetComponent*> Targets;
	GetComponents<UModelViewerFocusTargetComponent>(Targets);
	for (UModelViewerFocusTargetComponent* Target : Targets)
	{
		if (IsValid(Target) && !Target->TargetId.IsNone())
		{
			Result.AddUnique(Target->TargetId);
		}
	}

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors, true, true);
	for (AActor* AttachedActor : AttachedActors)
	{
		const AModelViewerFocusPointActor* FocusPoint =
			Cast<AModelViewerFocusPointActor>(AttachedActor);
		if (IsValid(FocusPoint) && !FocusPoint->TargetId.IsNone())
		{
			Result.AddUnique(FocusPoint->TargetId);
		}
	}
	return Result;
}

TArray<FName> AModelViewerDoorBase::GetModuleIds() const
{
	TArray<FName> Result = GetFocusTargetIds();
	for (const FModelViewerModuleDefinition& Definition : ModuleDefinitions)
	{
		if (!Definition.TargetId.IsNone())
		{
			Result.AddUnique(Definition.TargetId);
		}
	}
	return Result;
}

const FModelViewerModuleDefinition* AModelViewerDoorBase::FindModuleDefinition(
	FName TargetId) const
{
	return ModuleDefinitions.FindByPredicate(
		[TargetId](const FModelViewerModuleDefinition& Definition)
		{
			return Definition.TargetId == TargetId;
		});
}

TArray<UPrimitiveComponent*> AModelViewerDoorBase::GetModuleComponents(FName TargetId) const
{
	TArray<UPrimitiveComponent*> Result;
	if (TargetId.IsNone())
	{
		return Result;
	}

	FName ComponentTag = TargetId;
	if (const FModelViewerModuleDefinition* Definition = FindModuleDefinition(TargetId))
	{
		if (!Definition->ComponentTag.IsNone())
		{
			ComponentTag = Definition->ComponentTag;
		}
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* Component : PrimitiveComponents)
	{
		if (IsValid(Component) && Component->ComponentHasTag(ComponentTag))
		{
			Result.Add(Component);
		}
	}
	return Result;
}

void AModelViewerDoorBase::GetAllModuleComponents(
	TArray<UPrimitiveComponent*>& OutComponents) const
{
	OutComponents.Reset();
	for (const FName ModuleId : GetModuleIds())
	{
		for (UPrimitiveComponent* Component : GetModuleComponents(ModuleId))
		{
			OutComponents.AddUnique(Component);
		}
	}
}

void AModelViewerDoorBase::CapturePresentationState(UPrimitiveComponent* Component)
{
	if (!IsValid(Component))
	{
		return;
	}

	const bool bAlreadyCaptured = OriginalPresentationStates.ContainsByPredicate(
		[Component](const FModelViewerPrimitivePresentationState& State)
		{
			return State.Component == Component;
		});
	if (bAlreadyCaptured)
	{
		return;
	}

	FModelViewerPrimitivePresentationState& State = OriginalPresentationStates.AddDefaulted_GetRef();
	State.Component = Component;
	State.bVisible = Component->IsVisible();
	State.bRenderCustomDepth = Component->bRenderCustomDepth;
	State.CustomDepthStencilValue = Component->CustomDepthStencilValue;
	if (const UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component))
	{
		State.OverlayMaterial = MeshComponent->GetOverlayMaterial();
	}
}

void AModelViewerDoorBase::SetModuleMaterialValue(
	UPrimitiveComponent* Component, float Value) const
{
	if (ModuleMaterialParameterName.IsNone())
	{
		return;
	}

	if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component))
	{
		MeshComponent->SetScalarParameterValueOnMaterials(ModuleMaterialParameterName, Value);
	}
}

void AModelViewerDoorBase::RestoreModulePresentation()
{
	for (const FModelViewerPrimitivePresentationState& State : OriginalPresentationStates)
	{
		UPrimitiveComponent* Component = State.Component.Get();
		if (!IsValid(Component))
		{
			continue;
		}

		Component->SetVisibility(State.bVisible);
		Component->SetRenderCustomDepth(State.bRenderCustomDepth);
		Component->SetCustomDepthStencilValue(State.CustomDepthStencilValue);
		SetModuleMaterialValue(Component, NormalModuleMaterialValue);
		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component))
		{
			MeshComponent->SetOverlayMaterial(State.OverlayMaterial.Get());
		}
	}
}

void AModelViewerDoorBase::ApplyModulePresentation(FName TargetId)
{
	TArray<UPrimitiveComponent*> AllComponents;
	GetAllModuleComponents(AllComponents);
	for (UPrimitiveComponent* Component : AllComponents)
	{
		CapturePresentationState(Component);
	}

	RestoreModulePresentation();

	const TArray<UPrimitiveComponent*> SelectedComponents = GetModuleComponents(TargetId);
	for (UPrimitiveComponent* Component : AllComponents)
	{
		if (!IsValid(Component) || SelectedComponents.Contains(Component))
		{
			continue;
		}

		switch (OtherModulesEffect)
		{
		case EModelViewerOtherModuleEffect::MaterialParameter:
			SetModuleMaterialValue(Component, OtherModuleMaterialValue);
			break;
		case EModelViewerOtherModuleEffect::Hidden:
			Component->SetVisibility(false);
			break;
		case EModelViewerOtherModuleEffect::None:
		default:
			break;
		}
	}

	const FModelViewerModuleDefinition* Definition = FindModuleDefinition(TargetId);
	const bool bHighlight = !Definition || Definition->bHighlightWhenSelected;
	const int32 StencilValue = Definition
		? FMath::Clamp(Definition->HighlightStencilValue, 0, 255)
		: 1;
	for (UPrimitiveComponent* Component : SelectedComponents)
	{
		if (!IsValid(Component))
		{
			continue;
		}

		if (OtherModulesEffect == EModelViewerOtherModuleEffect::MaterialParameter)
		{
			SetModuleMaterialValue(Component, SelectedModuleMaterialValue);
		}
		if (bHighlight && bUseCustomDepthHighlight)
		{
			Component->SetRenderCustomDepth(true);
			Component->SetCustomDepthStencilValue(StencilValue);
		}
		if (bHighlight && IsValid(SelectedOverlayMaterial))
		{
			if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component))
			{
				MeshComponent->SetOverlayMaterial(SelectedOverlayMaterial);
			}
		}
	}
}

bool AModelViewerDoorBase::FocusModule(
	AModelViewerPawn* ViewerPawn, FName TargetId, bool bKeepCurrentYaw)
{
	if (!IsValid(ViewerPawn))
	{
		return false;
	}

	if (AModelViewerFocusPointActor* FocusPoint = FindFocusPointActor(TargetId))
	{
		ViewerPawn->FocusOnFocusPointActor(FocusPoint, bKeepCurrentYaw);
		return true;
	}

	if (UModelViewerFocusTargetComponent* Target = FindFocusTarget(TargetId))
	{
		ViewerPawn->FocusOnTarget(Target, bKeepCurrentYaw);
		return true;
	}

	return false;
}

bool AModelViewerDoorBase::SelectModule(
	AModelViewerPawn* ViewerPawn, FName TargetId, bool bKeepCurrentYaw)
{
	if (TargetId.IsNone() || !FocusModule(ViewerPawn, TargetId, bKeepCurrentYaw))
	{
		return false;
	}

	const FName PreviousTargetId = SelectedModuleId;
	if (!PreviousTargetId.IsNone() && PreviousTargetId != TargetId)
	{
		BP_OnModuleDeselected(PreviousTargetId, GetModuleComponents(PreviousTargetId));
	}

	SelectedModuleId = TargetId;
	ApplyModulePresentation(TargetId);
	BP_OnModuleSelected(TargetId, GetModuleComponents(TargetId));
	if (PreviousTargetId != TargetId)
	{
		OnModuleSelectionChanged.Broadcast(PreviousTargetId, TargetId);
	}
	return true;
}

void AModelViewerDoorBase::ClearModuleSelection()
{
	const FName PreviousTargetId = SelectedModuleId;
	if (!PreviousTargetId.IsNone())
	{
		BP_OnModuleDeselected(PreviousTargetId, GetModuleComponents(PreviousTargetId));
	}

	SelectedModuleId = NAME_None;
	RestoreModulePresentation();
	if (!PreviousTargetId.IsNone())
	{
		OnModuleSelectionChanged.Broadcast(PreviousTargetId, NAME_None);
	}
}

void AModelViewerDoorBase::SetDoorVisible(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(bVisible);
}
