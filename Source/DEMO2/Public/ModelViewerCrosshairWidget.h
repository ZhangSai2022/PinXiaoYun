#pragma once

#include "Blueprint/UserWidget.h"
#include "ModelViewerCrosshairWidget.generated.h"

/** Lightweight, code-drawn crosshair and hovered model label. */
UCLASS(Blueprintable, BlueprintType)
class DEMO2_API UModelViewerCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UModelViewerCrosshairWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Model Viewer|Crosshair")
	void SetTargetName(const FText& InTargetName);

	UFUNCTION(BlueprintPure, Category = "Model Viewer|Crosshair")
	FText GetTargetName() const { return TargetName; }

	void TickTargetNameAnimation(float DeltaTime);

	/** Optional TextBlock supplied by a derived Widget Blueprint. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Model Viewer|Crosshair")
	TObjectPtr<class UTextBlock> ModelNameText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Crosshair")
	FLinearColor CrosshairColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Crosshair")
	FLinearColor TargetNameColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Crosshair", meta = (ClampMin = "1.0"))
	float CrosshairLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairGap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Crosshair", meta = (ClampMin = "0.5"))
	float CrosshairThickness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Crosshair", meta = (ClampMin = "1.0"))
	int32 TargetNameFontSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Animation", meta = (ClampMin = "0.01"))
	float NameFadeInDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Animation", meta = (ClampMin = "0.01"))
	float NameFadeOutDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Viewer|Animation", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float NameStartScale;

protected:
	virtual void NativeConstruct() override;
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	UPROPERTY(Transient)
	FText TargetName;

	float NameAnimationElapsed = 0.0f;
	bool bNameAnimationShowing = false;
	bool bNameTextVisible = false;
};
