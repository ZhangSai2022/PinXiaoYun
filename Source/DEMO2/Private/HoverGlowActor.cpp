#include "HoverGlowActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "TimerManager.h"

AHoverGlowActor::AHoverGlowActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("HoverGlowRoot"));
	SetRootComponent(Root);
}

void AHoverGlowActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoEnableMouseOverEvents)
	{
		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
			PlayerController->bEnableMouseOverEvents = true;
			PlayerController->bEnableClickEvents = true;
		}
	}

	RefreshHoverMeshComponents();
}

void AHoverGlowActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreAndUnbindMeshes();
	Super::EndPlay(EndPlayReason);
}

void AHoverGlowActor::SetHoverGlowEnabled(bool bEnabled)
{
	if (bHoverGlowEnabled == bEnabled)
	{
		return;
	}

	bHoverGlowEnabled = bEnabled;
	if (!bHoverGlowEnabled)
	{
		const bool bWasHovered = IsHovered();
		HoveredMeshes.Reset();
		HoveredWidgets.Reset();
		WidgetOwnerMesh.Reset();
		GetWorldTimerManager().ClearTimer(HideWidgetsTimerHandle);
		ApplyGlowToAll(false);
		ApplyCustomDepthToAll(false);
		SetAllChildWidgetsVisible(false);
		if (bWasHovered)
		{
			NotifyHoverChanged(false, nullptr);
		}
	}
}

void AHoverGlowActor::RefreshHoverMeshComponents()
{
	const bool bWasHovered = IsHovered();
	RestoreAndUnbindMeshes();
	if (bWasHovered)
	{
		NotifyHoverChanged(false, nullptr);
	}

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
	{
		if (!IsValid(MeshComponent))
		{
			continue;
		}

		FHoverGlowMeshState& State = MeshStates.AddDefaulted_GetRef();
		State.MeshComponent = MeshComponent;
		State.OriginalOverlayMaterial = MeshComponent->GetOverlayMaterial();
		TArray<USceneComponent*> ChildComponents;
		MeshComponent->GetChildrenComponents(true, ChildComponents);
		for (USceneComponent* ChildComponent : ChildComponents)
		{
			if (UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(ChildComponent))
			{
				FHoverGlowWidgetState& WidgetState =
					State.ChildWidgetComponents.AddDefaulted_GetRef();
				WidgetState.WidgetComponent = WidgetComponent;
				WidgetState.OriginalVisibilityResponse =
					WidgetComponent->GetCollisionResponseToChannel(ECC_Visibility);
			}
		}
		for (const FHoverGlowWidgetState& WidgetState : State.ChildWidgetComponents)
		{
			UWidgetComponent* WidgetComponent = WidgetState.WidgetComponent.Get();
			if (IsValid(WidgetComponent))
			{
				WidgetComponent->SetCollisionResponseToChannel(
					ECC_Visibility,
					bChildWidgetsBlockVisibilityTrace ? ECR_Block : ECR_Ignore);
				WidgetComponent->SetVisibility(false, true);
				WidgetComponent->OnBeginCursorOver.AddUniqueDynamic(
					this, &AHoverGlowActor::HandleWidgetBeginCursorOver);
				WidgetComponent->OnEndCursorOver.AddUniqueDynamic(
					this, &AHoverGlowActor::HandleWidgetEndCursorOver);
			}
		}
		ApplyCustomDepth(MeshComponent, false);

		MeshComponent->OnBeginCursorOver.AddUniqueDynamic(
			this, &AHoverGlowActor::HandleBeginCursorOver);
		MeshComponent->OnEndCursorOver.AddUniqueDynamic(
			this, &AHoverGlowActor::HandleEndCursorOver);
		MeshComponent->OnClicked.AddUniqueDynamic(
			this, &AHoverGlowActor::HandleMeshClicked);
	}
}

void AHoverGlowActor::HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if (!bHoverGlowEnabled)
	{
		return;
	}

	UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(TouchedComponent);
	if (!IsValid(MeshComponent))
	{
		return;
	}

	const bool bWasHovered = IsHovered();
	HoveredMeshes.Add(MeshComponent);

	if (bHighlightWholeActor)
	{
		ApplyGlowToAll(true);
		ApplyCustomDepthToAll(true);
	}
	else
	{
		ApplyGlow(MeshComponent, true);
		ApplyCustomDepth(MeshComponent, true);
	}

	if (!bWasHovered)
	{
		NotifyHoverChanged(true, MeshComponent);
	}
}

void AHoverGlowActor::HandleEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(TouchedComponent);
	if (!IsValid(MeshComponent))
	{
		return;
	}

	if (HoveredMeshes.Remove(MeshComponent) == 0)
	{
		return;
	}

	RequestHideChildWidgets();
}

void AHoverGlowActor::HandleMeshClicked(
	UPrimitiveComponent* ClickedComponent, FKey ButtonPressed)
{
	if (!bHoverGlowEnabled)
	{
		return;
	}

	UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(ClickedComponent);
	if (bShowChildWidgetsOnClick && IsValid(MeshComponent) && HoveredMeshes.Contains(MeshComponent))
	{
		WidgetOwnerMesh = MeshComponent;
		SetChildWidgetsVisible(MeshComponent, true);
	}

	if (!IsValid(MeshComponent))
	{
		return;
	}

	const double CurrentTimeSeconds = FPlatformTime::Seconds();
	const bool bIsDoubleClick =
		LastClickedMesh == MeshComponent &&
		LastClickButton == ButtonPressed &&
		LastClickTimeSeconds >= 0.0 &&
		CurrentTimeSeconds - LastClickTimeSeconds <= DoubleClickInterval;

	if (bIsDoubleClick)
	{
		LastClickedMesh.Reset();
		LastClickButton = FKey();
		LastClickTimeSeconds = -1.0;
		OnStaticMeshDoubleClicked.Broadcast(MeshComponent);
		BP_OnStaticMeshDoubleClicked(MeshComponent);
	}
	else
	{
		LastClickedMesh = MeshComponent;
		LastClickButton = ButtonPressed;
		LastClickTimeSeconds = CurrentTimeSeconds;
	}
}

void AHoverGlowActor::HandleWidgetBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if (!bHoverGlowEnabled)
	{
		return;
	}

	UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(TouchedComponent);
	if (!IsValid(WidgetComponent))
	{
		return;
	}

	const bool bWasHovered = IsHovered();
	HoveredWidgets.Add(WidgetComponent);
	GetWorldTimerManager().ClearTimer(HideWidgetsTimerHandle);

	UStaticMeshComponent* OwnerMesh = FindOwnerMesh(WidgetComponent);
	if (bHighlightWholeActor)
	{
		ApplyGlowToAll(true);
		ApplyCustomDepthToAll(true);
	}
	else
	{
		ApplyGlow(OwnerMesh, true);
		ApplyCustomDepth(OwnerMesh, true);
	}

	if (!bWasHovered)
	{
		NotifyHoverChanged(true, OwnerMesh);
	}
}

void AHoverGlowActor::HandleWidgetEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(TouchedComponent);
	if (!IsValid(WidgetComponent))
	{
		return;
	}

	HoveredWidgets.Remove(WidgetComponent);
	RequestHideChildWidgets();
}

void AHoverGlowActor::ApplyGlow(UStaticMeshComponent* MeshComponent, bool bGlow) const
{
	if (!IsValid(MeshComponent))
	{
		return;
	}

	if (!GlowIntensityParameterName.IsNone())
	{
		MeshComponent->SetScalarParameterValueOnMaterials(
			GlowIntensityParameterName,
			bGlow ? HoverGlowIntensity : NormalGlowIntensity);
	}

	if (bGlow && !GlowColorParameterName.IsNone())
	{
		MeshComponent->SetVectorParameterValueOnMaterials(
			GlowColorParameterName,
			FVector(HoverGlowColor.R, HoverGlowColor.G, HoverGlowColor.B));
	}

	if (bGlow && IsValid(HoverOverlayMaterial))
	{
		MeshComponent->SetOverlayMaterial(HoverOverlayMaterial);
		return;
	}

	if (!bGlow)
	{
		const FHoverGlowMeshState* State = MeshStates.FindByPredicate(
			[MeshComponent](const FHoverGlowMeshState& Candidate)
			{
				return Candidate.MeshComponent == MeshComponent;
			});
		MeshComponent->SetOverlayMaterial(
			State ? State->OriginalOverlayMaterial.Get() : nullptr);
	}
}

void AHoverGlowActor::ApplyGlowToAll(bool bGlow) const
{
	for (const FHoverGlowMeshState& State : MeshStates)
	{
		ApplyGlow(State.MeshComponent.Get(), bGlow);
	}
}

void AHoverGlowActor::ApplyCustomDepth(
	UStaticMeshComponent* MeshComponent, bool bEnabled) const
{
	if (!IsValid(MeshComponent))
	{
		return;
	}

	MeshComponent->SetRenderCustomDepth(bUseCustomDepthOnHover && bEnabled);
	MeshComponent->SetCustomDepthStencilValue(
		FMath::Clamp(HoverCustomDepthStencilValue, 0, 255));
}

void AHoverGlowActor::ApplyCustomDepthToAll(bool bEnabled) const
{
	for (const FHoverGlowMeshState& State : MeshStates)
	{
		ApplyCustomDepth(State.MeshComponent.Get(), bEnabled);
	}
}

void AHoverGlowActor::SetChildWidgetsVisible(
	UStaticMeshComponent* MeshComponent, bool bVisible) const
{
	if (!IsValid(MeshComponent))
	{
		return;
	}

	const FHoverGlowMeshState* State = MeshStates.FindByPredicate(
		[MeshComponent](const FHoverGlowMeshState& Candidate)
		{
			return Candidate.MeshComponent == MeshComponent;
		});
	if (!State)
	{
		return;
	}

	for (const FHoverGlowWidgetState& WidgetState : State->ChildWidgetComponents)
	{
		UWidgetComponent* WidgetComponent = WidgetState.WidgetComponent.Get();
		if (IsValid(WidgetComponent))
		{
			WidgetComponent->SetVisibility(bVisible, true);
		}
	}
}

void AHoverGlowActor::SetAllChildWidgetsVisible(bool bVisible) const
{
	for (const FHoverGlowMeshState& State : MeshStates)
	{
		for (const FHoverGlowWidgetState& WidgetState : State.ChildWidgetComponents)
		{
			UWidgetComponent* WidgetComponent = WidgetState.WidgetComponent.Get();
			if (IsValid(WidgetComponent))
			{
				WidgetComponent->SetVisibility(bVisible, true);
			}
		}
	}
}

void AHoverGlowActor::RequestHideChildWidgets()
{
	GetWorldTimerManager().SetTimerForNextTick(
		this, &AHoverGlowActor::HideChildWidgetsIfOutside);
}

bool AHoverGlowActor::IsInteractionAreaHovered(UStaticMeshComponent* MeshComponent) const
{
	if (HoveredMeshes.Contains(MeshComponent))
	{
		return true;
	}

	for (const TWeakObjectPtr<UWidgetComponent>& HoveredWidget : HoveredWidgets)
	{
		if (FindOwnerMesh(HoveredWidget.Get()) == MeshComponent)
		{
			return true;
		}
	}
	return false;
}

void AHoverGlowActor::HideChildWidgetsIfOutside()
{
	const TWeakObjectPtr<UStaticMeshComponent> PreviousOwner = WidgetOwnerMesh;
	if (PreviousOwner.IsValid() &&
		!IsInteractionAreaHovered(PreviousOwner.Get()))
	{
		SetChildWidgetsVisible(PreviousOwner.Get(), false);
		WidgetOwnerMesh.Reset();
	}

	const bool bAnyInteractionAreaHovered = IsHovered();
	if (bHighlightWholeActor)
	{
		ApplyGlowToAll(bAnyInteractionAreaHovered);
		ApplyCustomDepthToAll(bAnyInteractionAreaHovered);
	}
	else
	{
		for (const FHoverGlowMeshState& State : MeshStates)
		{
			const bool bMeshHovered =
				IsInteractionAreaHovered(State.MeshComponent.Get());
			ApplyGlow(State.MeshComponent.Get(), bMeshHovered);
			ApplyCustomDepth(State.MeshComponent.Get(), bMeshHovered);
		}
	}

	if (!bAnyInteractionAreaHovered)
	{
		WidgetOwnerMesh.Reset();
		SetAllChildWidgetsVisible(false);
		NotifyHoverChanged(false, PreviousOwner.Get());
	}
}

UStaticMeshComponent* AHoverGlowActor::FindOwnerMesh(UWidgetComponent* WidgetComponent) const
{
	if (!IsValid(WidgetComponent))
	{
		return nullptr;
	}

	for (const FHoverGlowMeshState& State : MeshStates)
	{
		if (State.ChildWidgetComponents.ContainsByPredicate(
			[WidgetComponent](const FHoverGlowWidgetState& WidgetState)
			{
				return WidgetState.WidgetComponent == WidgetComponent;
			}))
		{
			return State.MeshComponent.Get();
		}
	}
	return nullptr;
}

void AHoverGlowActor::RestoreAndUnbindMeshes()
{
	for (const FHoverGlowMeshState& State : MeshStates)
	{
		UStaticMeshComponent* MeshComponent = State.MeshComponent.Get();
		if (!IsValid(MeshComponent))
		{
			continue;
		}

		ApplyGlow(MeshComponent, false);
		ApplyCustomDepth(MeshComponent, false);
		for (const FHoverGlowWidgetState& WidgetState : State.ChildWidgetComponents)
		{
			UWidgetComponent* WidgetComponent = WidgetState.WidgetComponent.Get();
			if (IsValid(WidgetComponent))
			{
				WidgetComponent->SetVisibility(false, true);
				WidgetComponent->SetCollisionResponseToChannel(
					ECC_Visibility, WidgetState.OriginalVisibilityResponse);
				WidgetComponent->OnBeginCursorOver.RemoveDynamic(
					this, &AHoverGlowActor::HandleWidgetBeginCursorOver);
				WidgetComponent->OnEndCursorOver.RemoveDynamic(
					this, &AHoverGlowActor::HandleWidgetEndCursorOver);
			}
		}
		MeshComponent->OnBeginCursorOver.RemoveDynamic(
			this, &AHoverGlowActor::HandleBeginCursorOver);
		MeshComponent->OnEndCursorOver.RemoveDynamic(
			this, &AHoverGlowActor::HandleEndCursorOver);
		MeshComponent->OnClicked.RemoveDynamic(
			this, &AHoverGlowActor::HandleMeshClicked);
	}

	HoveredMeshes.Reset();
	HoveredWidgets.Reset();
	WidgetOwnerMesh.Reset();
	GetWorldTimerManager().ClearTimer(HideWidgetsTimerHandle);
	MeshStates.Reset();
}

void AHoverGlowActor::NotifyHoverChanged(
	bool bIsHovered, UStaticMeshComponent* SourceComponent)
{
	OnHoverChanged.Broadcast(bIsHovered, SourceComponent);
	BP_OnHoverGlowChanged(bIsHovered, SourceComponent);
}
