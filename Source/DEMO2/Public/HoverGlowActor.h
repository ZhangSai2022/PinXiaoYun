// Base actor that adds mouse-hover glow to static meshes authored in child Blueprints.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoverGlowActor.generated.h"

class UMaterialInterface;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;
class UWidgetComponent;

USTRUCT()
struct FHoverGlowWidgetState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(Transient)
	TEnumAsByte<ECollisionResponse> OriginalVisibilityResponse = ECR_Block;
};

USTRUCT()
struct FHoverGlowMeshState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> OriginalOverlayMaterial;

	UPROPERTY(Transient)
	TArray<FHoverGlowWidgetState> ChildWidgetComponents;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHoverGlowChanged, bool, bIsHovered,
	UStaticMeshComponent*, SourceComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHoverGlowMeshDoubleClicked,
	UStaticMeshComponent*, MeshComponent);

UCLASS(Blueprintable, BlueprintType)
class DEMO2_API AHoverGlowActor : public AActor
{
	GENERATED_BODY()

public:
	AHoverGlowActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hover Glow")
	TObjectPtr<USceneComponent> Root;

	// The material should multiply this scalar into its Emissive Color output.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Material")
	FName GlowIntensityParameterName = TEXT("HoverGlowIntensity");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Material")
	float NormalGlowIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Material", meta = (ClampMin = "0.0"))
	float HoverGlowIntensity = 5.0f;

	// Optional vector parameter used by materials as the emissive tint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Material")
	FName GlowColorParameterName = TEXT("HoverGlowColor");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Material")
	FLinearColor HoverGlowColor = FLinearColor(1.0f, 0.45f, 0.05f, 1.0f);

	// Optional emissive overlay. This also works when the mesh material has no glow parameters.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Material")
	TObjectPtr<UMaterialInterface> HoverOverlayMaterial;

	// When true, hovering any mesh highlights every static mesh in this actor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Behavior")
	bool bHighlightWholeActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Behavior")
	bool bUseCustomDepthOnHover = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Behavior", meta = (ClampMin = "0", ClampMax = "255"))
	int32 HoverCustomDepthStencilValue = 1;

	// Enables cursor-over tracing on player zero. The mesh must still block Visibility traces.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Behavior")
	bool bAutoEnableMouseOverEvents = true;

	// Child WidgetComponents start hidden, appear when their mesh is clicked while hovered,
	// and hide again as soon as the cursor leaves that mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Behavior")
	bool bShowChildWidgetsOnClick = true;

	// WidgetComponents must block Visibility to count as part of the hovered interaction area.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Behavior")
	bool bChildWidgetsBlockVisibilityTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Glow|Behavior", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float DoubleClickInterval = 0.3f;

	UPROPERTY(BlueprintAssignable, Category = "Hover Glow")
	FOnHoverGlowChanged OnHoverChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hover Glow")
	FOnHoverGlowMeshDoubleClicked OnStaticMeshDoubleClicked;

	UFUNCTION(BlueprintPure, Category = "Hover Glow")
	bool IsHovered() const { return HoveredMeshes.Num() > 0 || HoveredWidgets.Num() > 0; }

	UFUNCTION(BlueprintCallable, Category = "Hover Glow")
	void SetHoverGlowEnabled(bool bEnabled);

	// Call this after adding static mesh components dynamically at runtime.
	UFUNCTION(BlueprintCallable, Category = "Hover Glow")
	void RefreshHoverMeshComponents();

	UFUNCTION(BlueprintImplementableEvent, Category = "Hover Glow", meta = (DisplayName = "On Hover Glow Changed"))
	void BP_OnHoverGlowChanged(bool bIsHovered, UStaticMeshComponent* SourceComponent);

	// Implement this event in a child Blueprint to react to a static mesh double click.
	UFUNCTION(BlueprintImplementableEvent, Category = "Hover Glow", meta = (DisplayName = "On Static Mesh Double Clicked"))
	void BP_OnStaticMeshDoubleClicked(UStaticMeshComponent* MeshComponent);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleMeshClicked(UPrimitiveComponent* ClickedComponent, FKey ButtonPressed);

	UFUNCTION()
	void HandleWidgetBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleWidgetEndCursorOver(UPrimitiveComponent* TouchedComponent);

	void ApplyGlow(UStaticMeshComponent* MeshComponent, bool bGlow) const;
	void ApplyGlowToAll(bool bGlow) const;
	void ApplyCustomDepth(UStaticMeshComponent* MeshComponent, bool bEnabled) const;
	void ApplyCustomDepthToAll(bool bEnabled) const;
	void SetChildWidgetsVisible(UStaticMeshComponent* MeshComponent, bool bVisible) const;
	void SetAllChildWidgetsVisible(bool bVisible) const;
	void RequestHideChildWidgets();
	void HideChildWidgetsIfOutside();
	UStaticMeshComponent* FindOwnerMesh(UWidgetComponent* WidgetComponent) const;
	bool IsInteractionAreaHovered(UStaticMeshComponent* MeshComponent) const;
	void RestoreAndUnbindMeshes();
	void NotifyHoverChanged(bool bIsHovered, UStaticMeshComponent* SourceComponent);

	UPROPERTY(Transient)
	TArray<FHoverGlowMeshState> MeshStates;

	TSet<TWeakObjectPtr<UStaticMeshComponent>> HoveredMeshes;
	TSet<TWeakObjectPtr<UWidgetComponent>> HoveredWidgets;
	TWeakObjectPtr<UStaticMeshComponent> WidgetOwnerMesh;
	TWeakObjectPtr<UStaticMeshComponent> LastClickedMesh;
	FKey LastClickButton;
	double LastClickTimeSeconds = -1.0;
	FTimerHandle HideWidgetsTimerHandle;
	bool bHoverGlowEnabled = true;
};
