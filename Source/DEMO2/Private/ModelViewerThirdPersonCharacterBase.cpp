#include "ModelViewerThirdPersonCharacterBase.h"

#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "ModelViewerCrosshairWidget.h"
#include "ModelViewerDoorBase.h"

AModelViewerThirdPersonCharacterBase::AModelViewerThirdPersonCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	CrosshairWidgetClass = UModelViewerCrosshairWidget::StaticClass();
	ModelTraceChannel = ECC_Visibility;
	ModelTraceDistance = 5000.0f;
	HoverHighlightStencilValue = 1;
	bUseCustomDepthHighlight = true;
	bDrawModelTrace = false;
	bUseHandCursorOnHover = true;
	bRequireClickedModelToBeFocused = true;
	bToggleDescriptionOnClick = true;
	DescriptionFadeInDuration = 0.32f;
	DescriptionFadeOutDuration = 0.18f;
	DescriptionStartScale = 0.76f;
}

void AModelViewerThirdPersonCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableMouseOverEvents = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		if (CrosshairWidgetClass)
		{
			CrosshairWidget = CreateWidget<UModelViewerCrosshairWidget>(
				PlayerController, CrosshairWidgetClass);
			if (CrosshairWidget)
			{
				CrosshairWidget->SetTargetName(FText::GetEmpty());
				CrosshairWidget->AddToViewport(100);
			}
		}
	}
}

void AModelViewerThirdPersonCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PerformModelTrace();
	UpdateMouseInteraction();
	if (CrosshairWidget)
	{
		CrosshairWidget->TickTargetNameAnimation(DeltaSeconds);
	}
	UpdateDescriptionAnimations(DeltaSeconds);
}

void AModelViewerThirdPersonCharacterBase::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindKey(
			EKeys::LeftMouseButton, IE_Pressed,
			this, &AModelViewerThirdPersonCharacterBase::InteractWithHoveredModel);
	}
}

void AModelViewerThirdPersonCharacterBase::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerController* PlayerController = GetWorld()
		? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		UpdateHoverCursor(PlayerController, false);
	}
	Super::EndPlay(EndPlayReason);
}

void AModelViewerThirdPersonCharacterBase::PerformModelTrace()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}
	UWorld* World = GetWorld();
	if (!PlayerController || !World)
	{
		ClearModelHover();
		return;
	}

	FVector TraceStart;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(TraceStart, ViewRotation);
	const FVector TraceEnd =
		TraceStart + ViewRotation.Vector() * ModelTraceDistance;
	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(ModelViewerCrosshair), true, this);
	QueryParams.bTraceComplex = true;
	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit, TraceStart, TraceEnd, ModelTraceChannel, QueryParams);

	if (bDrawModelTrace && !TraceStart.IsNearlyZero())
	{
		DrawDebugLine(World, TraceStart, bHit ? Hit.ImpactPoint : TraceEnd,
			bHit ? FColor::Green : FColor::Red, false, 0.0f, 0, 1.0f);
	}

	UStaticMeshComponent* NewComponent = bHit
		? GetValidModelComponent(Hit) : nullptr;
	FText NewName;
	if (NewComponent)
	{
		NewName = GetModelTagName(NewComponent);
	}

	if (NewComponent != HoveredModelComponent)
	{
		ApplyHover(NewComponent, NewName);
	}
}

void AModelViewerThirdPersonCharacterBase::InteractWithHoveredModel()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}
	UStaticMeshComponent* ClickedComponent =
		TraceModelUnderMouse(PlayerController);
	if (!ClickedComponent ||
		(bRequireClickedModelToBeFocused &&
			ClickedComponent != HoveredModelComponent))
	{
		return;
	}

	UWidgetComponent* DescriptionWidget =
		FindDescriptionWidgetComponent(ClickedComponent);
	if (!DescriptionWidget)
	{
		return;
	}

	if (DescriptionWidget == ActiveDescriptionWidgetComponent &&
		bToggleDescriptionOnClick)
	{
		HideComponentDescription();
		return;
	}

	BeginShowDescription(DescriptionWidget);
}

UStaticMeshComponent* AModelViewerThirdPersonCharacterBase::TraceModelUnderMouse(
	APlayerController* PlayerController) const
{
	UWorld* World = GetWorld();
	if (!PlayerController || !World || !PlayerController->bShowMouseCursor)
	{
		return nullptr;
	}

	FVector TraceStart;
	FVector MouseDirection;
	if (!PlayerController->DeprojectMousePositionToWorld(
		TraceStart, MouseDirection))
	{
		return nullptr;
	}

	const FVector TraceEnd =
		TraceStart + MouseDirection * ModelTraceDistance;
	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(ModelViewerMouseInteraction), true, this);
	QueryParams.bTraceComplex = true;
	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(
		Hit, TraceStart, TraceEnd, ModelTraceChannel, QueryParams))
	{
		return nullptr;
	}
	return GetValidModelComponent(Hit);
}

UStaticMeshComponent* AModelViewerThirdPersonCharacterBase::GetValidModelComponent(
	const FHitResult& Hit) const
{
	UStaticMeshComponent* Component =
		Cast<UStaticMeshComponent>(Hit.GetComponent());
	return Component && Cast<AModelViewerDoorBase>(Component->GetOwner())
		? Component : nullptr;
}

void AModelViewerThirdPersonCharacterBase::UpdateMouseInteraction()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}

	MouseHoveredModelComponent = TraceModelUnderMouse(PlayerController);
	const bool bCanInteract = MouseHoveredModelComponent &&
		(!bRequireClickedModelToBeFocused ||
			MouseHoveredModelComponent == HoveredModelComponent);
	UpdateHoverCursor(PlayerController, bCanInteract);
}

void AModelViewerThirdPersonCharacterBase::HideComponentDescription()
{
	if (ActiveDescriptionWidgetComponent)
	{
		BeginHideDescription(ActiveDescriptionWidgetComponent);
	}
}

UWidgetComponent* AModelViewerThirdPersonCharacterBase::FindDescriptionWidgetComponent(
	UStaticMeshComponent* Component) const
{
	if (!Component)
	{
		return nullptr;
	}

	TArray<USceneComponent*> AttachedComponents;
	Component->GetChildrenComponents(true, AttachedComponents);
	for (USceneComponent* AttachedComponent : AttachedComponents)
	{
		if (UWidgetComponent* WidgetComponent =
			Cast<UWidgetComponent>(AttachedComponent))
		{
			return WidgetComponent;
		}
	}

	AActor* OwnerActor = Component->GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	TArray<UWidgetComponent*> WidgetComponents;
	OwnerActor->GetComponents<UWidgetComponent>(WidgetComponents);
	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		for (USceneComponent* Parent = WidgetComponent
			? WidgetComponent->GetAttachParent() : nullptr;
			Parent;
			Parent = Parent->GetAttachParent())
		{
			if (Parent == Component)
			{
				return WidgetComponent;
			}
		}
	}

	return nullptr;
}

void AModelViewerThirdPersonCharacterBase::BeginShowDescription(
	UWidgetComponent* WidgetComponent)
{
	if (!WidgetComponent)
	{
		return;
	}

	if (ActiveDescriptionWidgetComponent &&
		ActiveDescriptionWidgetComponent != WidgetComponent)
	{
		BeginHideDescription(ActiveDescriptionWidgetComponent);
	}
	if (FadingDescriptionWidgetComponent == WidgetComponent)
	{
		FadingDescriptionWidgetComponent = nullptr;
	}

	ActiveDescriptionWidgetComponent = WidgetComponent;
	DescriptionFadeInElapsed = 0.0f;
	WidgetComponent->SetVisibility(true, true);
	WidgetComponent->InitWidget();
	ApplyDescriptionAnimation(WidgetComponent, 0.0f, DescriptionStartScale);
}

void AModelViewerThirdPersonCharacterBase::BeginHideDescription(
	UWidgetComponent* WidgetComponent)
{
	if (!WidgetComponent)
	{
		return;
	}

	if (FadingDescriptionWidgetComponent &&
		FadingDescriptionWidgetComponent != WidgetComponent)
	{
		FadingDescriptionWidgetComponent->SetVisibility(false, true);
	}

	FadingDescriptionWidgetComponent = WidgetComponent;
	DescriptionFadeOutElapsed = 0.0f;
	if (ActiveDescriptionWidgetComponent == WidgetComponent)
	{
		ActiveDescriptionWidgetComponent = nullptr;
	}
}

void AModelViewerThirdPersonCharacterBase::UpdateDescriptionAnimations(
	float DeltaTime)
{
	const float SafeDelta = FMath::Max(DeltaTime, 0.0f);
	if (ActiveDescriptionWidgetComponent)
	{
		DescriptionFadeInElapsed += SafeDelta;
		const float Alpha = FMath::Clamp(
			DescriptionFadeInElapsed /
			FMath::Max(DescriptionFadeInDuration, 0.01f), 0.0f, 1.0f);
		const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
		const float X = Alpha - 1.0f;
		const float EaseOutBack =
			1.0f + 2.70158f * X * X * X + 1.70158f * X * X;
		ApplyDescriptionAnimation(
			ActiveDescriptionWidgetComponent,
			SmoothAlpha,
			FMath::Lerp(DescriptionStartScale, 1.0f, EaseOutBack));
	}

	if (FadingDescriptionWidgetComponent)
	{
		DescriptionFadeOutElapsed += SafeDelta;
		const float Alpha = FMath::Clamp(
			DescriptionFadeOutElapsed /
			FMath::Max(DescriptionFadeOutDuration, 0.01f), 0.0f, 1.0f);
		const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
		ApplyDescriptionAnimation(
			FadingDescriptionWidgetComponent,
			1.0f - SmoothAlpha,
			FMath::Lerp(1.0f, 0.92f, SmoothAlpha));
		if (Alpha >= 1.0f)
		{
			FadingDescriptionWidgetComponent->SetVisibility(false, true);
			FadingDescriptionWidgetComponent = nullptr;
		}
	}
}

void AModelViewerThirdPersonCharacterBase::ApplyDescriptionAnimation(
	UWidgetComponent* WidgetComponent, float Opacity, float Scale) const
{
	if (!WidgetComponent)
	{
		return;
	}
	WidgetComponent->InitWidget();
	if (UUserWidget* DescriptionWidget =
		WidgetComponent->GetUserWidgetObject())
	{
		DescriptionWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		DescriptionWidget->SetRenderOpacity(FMath::Clamp(Opacity, 0.0f, 1.0f));
		DescriptionWidget->SetRenderScale(FVector2D(Scale));
	}
}

void AModelViewerThirdPersonCharacterBase::UpdateHoverCursor(
	APlayerController* PlayerController, bool bHoveringModel)
{
	if (!PlayerController || !bUseHandCursorOnHover)
	{
		return;
	}

	if (bHoveringModel && !bHandCursorActive)
	{
		CursorBeforeHover = PlayerController->CurrentMouseCursor;
		PlayerController->CurrentMouseCursor = EMouseCursor::Hand;
		bHandCursorActive = true;
	}
	else if (!bHoveringModel && bHandCursorActive)
	{
		PlayerController->CurrentMouseCursor = CursorBeforeHover;
		bHandCursorActive = false;
	}
}

void AModelViewerThirdPersonCharacterBase::ApplyHover(
	UStaticMeshComponent* NewComponent, const FText& NewName)
{
	RestoreHoveredComponent();

	HoveredModelComponent = NewComponent;
	HoveredModelName = NewComponent ? NewName : FText::GetEmpty();
	if (NewComponent)
	{
		bPreviousRenderCustomDepth = NewComponent->bRenderCustomDepth;
		PreviousCustomDepthStencilValue = NewComponent->CustomDepthStencilValue;
		PreviousOverlayMaterial = NewComponent->GetOverlayMaterial();
		if (bUseCustomDepthHighlight)
		{
			NewComponent->SetRenderCustomDepth(true);
			NewComponent->SetCustomDepthStencilValue(
				FMath::Clamp(HoverHighlightStencilValue, 0, 255));
		}
		if (HoverOverlayMaterial)
		{
			NewComponent->SetOverlayMaterial(HoverOverlayMaterial);
		}
	}

	if (CrosshairWidget)
	{
		CrosshairWidget->SetTargetName(HoveredModelName);
	}
	OnModelViewerHoverChanged.Broadcast(NewComponent, HoveredModelName);
	BP_OnModelViewerHoverChanged(NewComponent, HoveredModelName);
}

void AModelViewerThirdPersonCharacterBase::RestoreHoveredComponent()
{
	if (!HoveredModelComponent)
	{
		return;
	}

	HoveredModelComponent->SetRenderCustomDepth(bPreviousRenderCustomDepth);
	HoveredModelComponent->SetCustomDepthStencilValue(PreviousCustomDepthStencilValue);
	HoveredModelComponent->SetOverlayMaterial(PreviousOverlayMaterial.Get());
	PreviousOverlayMaterial = nullptr;
}

void AModelViewerThirdPersonCharacterBase::ClearModelHover()
{
	if (!HoveredModelComponent && HoveredModelName.IsEmpty())
	{
		return;
	}
	ApplyHover(nullptr, FText::GetEmpty());
}

FText AModelViewerThirdPersonCharacterBase::GetModelTagName(
	const UStaticMeshComponent* Component) const
{
	for (const USceneComponent* TaggedComponent = Component;
		TaggedComponent;
		TaggedComponent = TaggedComponent->GetAttachParent())
	{
		for (const FName& ComponentTag : TaggedComponent->ComponentTags)
		{
			if (!ComponentTag.IsNone())
			{
				return FText::FromName(ComponentTag);
			}
		}
	}

	if (Component)
	{
		if (const AActor* OwnerActor = Component->GetOwner())
		{
			for (const FName& ActorTag : OwnerActor->Tags)
			{
				if (!ActorTag.IsNone())
				{
					return FText::FromName(ActorTag);
				}
			}
		}
	}

	return FText::GetEmpty();
}
