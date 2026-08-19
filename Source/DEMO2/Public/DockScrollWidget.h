#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DockScrollItemWidget.h"
#include "DockScrollWidget.generated.h"

class UDockScrollItemWidget;
class UScrollBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FDockSelectionChangedSignature,
	int32, SelectedIndex,
	FName, ItemId,
	FText, DisplayName);

/** Horizontal, Apple Dock-inspired selector with wheel and right-drag scrolling. */
UCLASS(Blueprintable, BlueprintType)
class DEMO2_API UDockScrollWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dock")
	void InitializeDock(
		const TArray<FDockScrollItemData>& InItems,
		TSubclassOf<UDockScrollItemWidget> InItemWidgetClass,
		int32 InitialIndex = -1);

	UFUNCTION(BlueprintCallable, Category = "Dock")
	void SetItems(const TArray<FDockScrollItemData>& InItems);

	UFUNCTION(BlueprintCallable, Category = "Dock")
	void RefreshItems();

	UFUNCTION(BlueprintCallable, Category = "Dock")
	void SelectIndex(int32 Index, bool bBroadcastEvent = true);

	UFUNCTION(BlueprintCallable, Category = "Dock")
	void SelectNext();

	UFUNCTION(BlueprintCallable, Category = "Dock")
	void SelectPrevious();

	UFUNCTION(BlueprintPure, Category = "Dock")
	int32 GetSelectedIndex() const { return SelectedIndex; }

	UFUNCTION(BlueprintPure, Category = "Dock")
	int32 GetCurrentIndex() const { return CurrentIndex; }

	UPROPERTY(BlueprintAssignable, Category = "Dock|Events")
	FDockSelectionChangedSignature OnItemSelected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Content", meta = (ExposeOnSpawn = "true"))
	TArray<FDockScrollItemData> Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Content", meta = (ExposeOnSpawn = "true"))
	TSubclassOf<UDockScrollItemWidget> ItemWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Layout", meta = (ClampMin = "64.0"))
	float ItemWidth = 176.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Layout", meta = (ClampMin = "64.0"))
	float ItemHeight = 194.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Layout", meta = (ClampMin = "0.0"))
	float ItemSpacing = 44.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Motion", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float MaximumScale = 1.38f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Motion", meta = (ClampMin = "1.0"))
	float MagnificationRadius = 255.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Motion", meta = (ClampMin = "0.0"))
	float LiftAtMaximumScale = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Motion", meta = (ClampMin = "0.0"))
	float ScaleInterpolationSpeed = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock|Motion", meta = (ClampMin = "1.0"))
	float WheelStep = 180.0f;


protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	void SetHoveredItem(int32 Index);
	void ClearHoveredItem(int32 Index);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dock")
	TObjectPtr<UScrollBox> DockScrollBox;

private:
	friend class UDockScrollItemWidget;

	void BuildDefaultWidgetTree();
	void UpdateMagnification(float DeltaTime);
	void ScrollItemIntoCenter(int32 Index);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDockScrollItemWidget>> SpawnedItems;

	int32 SelectedIndex = INDEX_NONE;
	int32 CurrentIndex = INDEX_NONE;
	int32 HoveredIndex = INDEX_NONE;
	bool bPendingInitialSelection = false;
};
