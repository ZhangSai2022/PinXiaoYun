// Blueprintable base actor for a door/product model composed of multiple modules.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModelViewerDoorBase.generated.h"

class AModelViewerPawn;
class AModelViewerFocusPointActor;
class UModelViewerFocusTargetComponent;
class UMaterialInterface;
class UPrimitiveComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class EModelViewerOtherModuleEffect : uint8
{
	None UMETA(DisplayName = "Keep Normal"),
	MaterialParameter UMETA(DisplayName = "Material Parameter"),
	Hidden UMETA(DisplayName = "Hidden")
};

USTRUCT(BlueprintType)
struct DEMO2_API FModelViewerModuleDefinition
{
	GENERATED_BODY()

	// Must match the TargetId on the module's focus point.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Module")
	FName TargetId;

	// Mesh components with this Component Tag belong to the module. None uses TargetId.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Module")
	FName ComponentTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Module")
	bool bHighlightWhenSelected = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Module", meta = (ClampMin = "0", ClampMax = "255"))
	int32 HighlightStencilValue = 1;
};

USTRUCT()
struct FModelViewerPrimitivePresentationState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> Component;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> OverlayMaterial;

	bool bVisible = true;
	bool bRenderCustomDepth = false;
	int32 CustomDepthStencilValue = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnModelViewerModuleSelectionChanged, FName, PreviousTargetId, FName, NewTargetId);

UCLASS(Blueprintable, BlueprintType)
class DEMO2_API AModelViewerDoorBase : public AActor
{
	GENERATED_BODY()

public:
	AModelViewerDoorBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Model")
	TObjectPtr<USceneComponent> DoorRoot;

	// Add static meshes, skeletal meshes, collision and effects under this node.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Model")
	TObjectPtr<USceneComponent> ModelRoot;

	// Keep focus marker components under this node to make the Blueprint hierarchy clear.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Model")
	TObjectPtr<USceneComponent> FocusTargetRoot;

	// Add one entry per selectable part. ComponentTag may be left empty when it equals TargetId.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Model|Modules")
	TArray<FModelViewerModuleDefinition> ModuleDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Model|Modules")
	EModelViewerOtherModuleEffect OtherModulesEffect =
		EModelViewerOtherModuleEffect::MaterialParameter;

	// Materials that expose this scalar can dim or fade non-selected modules.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Model|Modules", meta = (EditCondition = "OtherModulesEffect == EModelViewerOtherModuleEffect::MaterialParameter"))
	FName ModuleMaterialParameterName = TEXT("ViewerOpacity");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Model|Modules", meta = (EditCondition = "OtherModulesEffect == EModelViewerOtherModuleEffect::MaterialParameter"))
	float NormalModuleMaterialValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Model|Modules", meta = (EditCondition = "OtherModulesEffect == EModelViewerOtherModuleEffect::MaterialParameter"))
	float SelectedModuleMaterialValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Model|Modules", meta = (EditCondition = "OtherModulesEffect == EModelViewerOtherModuleEffect::MaterialParameter"))
	float OtherModuleMaterialValue = 0.15f;

	// Optional visible highlight material applied through MeshComponent Overlay Material.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Model|Modules")
	TObjectPtr<UMaterialInterface> SelectedOverlayMaterial;

	// Custom depth supports a post-process outline when Custom Depth-Stencil is enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Model|Modules")
	bool bUseCustomDepthHighlight = true;

	UPROPERTY(BlueprintAssignable, Category = "Door Model|Modules")
	FOnModelViewerModuleSelectionChanged OnModuleSelectionChanged;

	UFUNCTION(BlueprintPure, Category = "Door Model|Focus")
	UModelViewerFocusTargetComponent* FindFocusTarget(FName TargetId) const;

	UFUNCTION(BlueprintPure, Category = "Door Model|Focus")
	AModelViewerFocusPointActor* FindFocusPointActor(FName TargetId) const;

	UFUNCTION(BlueprintPure, Category = "Door Model|Focus")
	TArray<FName> GetFocusTargetIds() const;

	UFUNCTION(BlueprintPure, Category = "Door Model|Modules")
	TArray<FName> GetModuleIds() const;

	UFUNCTION(BlueprintPure, Category = "Door Model|Modules")
	TArray<UPrimitiveComponent*> GetModuleComponents(FName TargetId) const;

	UFUNCTION(BlueprintPure, Category = "Door Model|Modules")
	FName GetSelectedModuleId() const { return SelectedModuleId; }

	UFUNCTION(BlueprintCallable, Category = "Door Model|Focus")
	bool FocusModule(AModelViewerPawn* ViewerPawn, FName TargetId, bool bKeepCurrentYaw);

	// Focuses the camera, updates all module presentation states, then calls the Blueprint event.
	UFUNCTION(BlueprintCallable, Category = "Door Model|Modules")
	bool SelectModule(AModelViewerPawn* ViewerPawn, FName TargetId, bool bKeepCurrentYaw);

	UFUNCTION(BlueprintCallable, Category = "Door Model|Modules")
	void ClearModuleSelection();

	UFUNCTION(BlueprintImplementableEvent, Category = "Door Model|Modules", meta = (DisplayName = "On Module Selected"))
	void BP_OnModuleSelected(FName TargetId, const TArray<UPrimitiveComponent*>& ModuleComponents);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door Model|Modules", meta = (DisplayName = "On Module Deselected"))
	void BP_OnModuleDeselected(FName TargetId, const TArray<UPrimitiveComponent*>& ModuleComponents);

	UFUNCTION(BlueprintCallable, Category = "Door Model|Focus")
	void SetDoorVisible(bool bVisible);

protected:
	virtual void BeginPlay() override;

private:
	const FModelViewerModuleDefinition* FindModuleDefinition(FName TargetId) const;
	void GetAllModuleComponents(TArray<UPrimitiveComponent*>& OutComponents) const;
	void CapturePresentationState(UPrimitiveComponent* Component);
	void RestoreModulePresentation();
	void ApplyModulePresentation(FName TargetId);
	void SetModuleMaterialValue(UPrimitiveComponent* Component, float Value) const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Door Model|Modules", meta = (AllowPrivateAccess = "true"))
	FName SelectedModuleId;

	UPROPERTY(Transient)
	TArray<FModelViewerPrimitivePresentationState> OriginalPresentationStates;
};
