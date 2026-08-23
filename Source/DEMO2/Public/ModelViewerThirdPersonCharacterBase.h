#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ModelViewerThirdPersonCharacterBase.generated.h"

class UModelViewerCrosshairWidget;
class UMaterialInterface;
class UStaticMeshComponent;
class UUserWidget;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnModelViewerHoverChanged,
	UStaticMeshComponent*, HoveredComponent,
	FText, ModelName);

/**
 * Reusable Character base for model-viewer scenes.
 * The mouse cursor (or camera center when configured) is traced every frame.
 * Only static meshes owned by AModelViewerDoorBase can become hovered.
 */
UCLASS(Blueprintable, BlueprintType)
class DEMO2_API AModelViewerThirdPersonCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AModelViewerThirdPersonCharacterBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Model Viewer|Crosshair")
	TSubclassOf<UModelViewerCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Trace")
	TEnumAsByte<ECollisionChannel> ModelTraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Trace", meta = (ClampMin = "1.0"))
	float ModelTraceDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Highlight", meta = (ClampMin = "0", ClampMax = "255"))
	int32 HoverHighlightStencilValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Highlight")
	bool bUseCustomDepthHighlight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Highlight")
	TObjectPtr<UMaterialInterface> HoverOverlayMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Trace")
	bool bDrawModelTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Interaction")
	bool bUseHandCursorOnHover;

	// Mouse clicks only interact with the exact component focused by the crosshair.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Interaction")
	bool bRequireClickedModelToBeFocused;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Interaction")
	bool bToggleDescriptionOnClick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Description Animation", meta = (ClampMin = "0.01"))
	float DescriptionFadeInDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Description Animation", meta = (ClampMin = "0.01"))
	float DescriptionFadeOutDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Description Animation", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float DescriptionStartScale;

	UPROPERTY(BlueprintReadOnly, Category = "Model Viewer|Trace")
	TObjectPtr<UStaticMeshComponent> HoveredModelComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Model Viewer|Trace")
	FText HoveredModelName;

	UPROPERTY(BlueprintReadOnly, Category = "Model Viewer|Interaction")
	TObjectPtr<UWidgetComponent> ActiveDescriptionWidgetComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Model Viewer|Interaction")
	TObjectPtr<UStaticMeshComponent> MouseHoveredModelComponent;

	UPROPERTY(BlueprintAssignable, Category = "Model Viewer|Trace")
	FOnModelViewerHoverChanged OnModelViewerHoverChanged;

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Trace")
	void PerformModelTrace();

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Trace")
	void ClearModelHover();

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Trace")
	FText GetHoveredModelName() const { return HoveredModelName; }

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Interaction")
	void InteractWithHoveredModel();

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Interaction")
	void HideComponentDescription();

	UFUNCTION(BlueprintImplementableEvent, Category = "Model Viewer|Trace", meta = (DisplayName = "On Model Hover Changed"))
	void BP_OnModelViewerHoverChanged(UStaticMeshComponent* Component, const FText& ModelName);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyHover(UStaticMeshComponent* NewComponent, const FText& NewName);
	void RestoreHoveredComponent();
	FText GetModelTagName(const UStaticMeshComponent* Component) const;
	UStaticMeshComponent* TraceModelUnderMouse(class APlayerController* PlayerController) const;
	UStaticMeshComponent* GetValidModelComponent(const FHitResult& Hit) const;
	void UpdateMouseInteraction();
	UWidgetComponent* FindDescriptionWidgetComponent(UStaticMeshComponent* Component) const;
	void BeginShowDescription(UWidgetComponent* WidgetComponent);
	void BeginHideDescription(UWidgetComponent* WidgetComponent);
	void UpdateDescriptionAnimations(float DeltaTime);
	void ApplyDescriptionAnimation(UWidgetComponent* WidgetComponent, float Opacity, float Scale) const;
	void UpdateHoverCursor(class APlayerController* PlayerController, bool bHoveringModel);

	UPROPERTY(Transient)
	TObjectPtr<UModelViewerCrosshairWidget> CrosshairWidget;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> PreviousOverlayMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> FadingDescriptionWidgetComponent;

	bool bPreviousRenderCustomDepth = false;
	int32 PreviousCustomDepthStencilValue = 0;
	float DescriptionFadeInElapsed = 0.0f;
	float DescriptionFadeOutElapsed = 0.0f;
	bool bHandCursorActive = false;
	TEnumAsByte<EMouseCursor::Type> CursorBeforeHover = EMouseCursor::Default;
};
