#include "ModelViewerCrosshairWidget.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"

UModelViewerCrosshairWidget::UModelViewerCrosshairWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CrosshairColor(FLinearColor(0.2f, 0.95f, 1.0f, 0.95f))
	, TargetNameColor(FLinearColor::White)
	, CrosshairLength(10.0f)
	, CrosshairGap(5.0f)
	, CrosshairThickness(2.0f)
	, TargetNameFontSize(18)
	, NameFadeInDuration(0.28f)
	, NameFadeOutDuration(0.16f)
	, NameStartScale(0.78f)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UModelViewerCrosshairWidget::SetTargetName(const FText& InTargetName)
{
	TargetName = InTargetName;
	if (!ModelNameText)
	{
		return;
	}

	NameAnimationElapsed = 0.0f;
	bNameAnimationShowing = !TargetName.IsEmpty();
	if (bNameAnimationShowing)
	{
		ModelNameText->SetText(TargetName);
		ModelNameText->SetVisibility(ESlateVisibility::HitTestInvisible);
		bNameTextVisible = true;
	}
	InvalidateLayoutAndVolatility();
}

void UModelViewerCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ModelNameText)
	{
		ModelNameText->SetColorAndOpacity(TargetNameColor);
		ModelNameText->SetJustification(ETextJustify::Left);
		ModelNameText->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		ModelNameText->SetRenderOpacity(0.0f);
		ModelNameText->SetRenderScale(FVector2D(NameStartScale));
		ModelNameText->SetVisibility(ESlateVisibility::Collapsed);
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ModelNameText->Slot))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
			CanvasSlot->SetPosition(FVector2D(CrosshairGap + CrosshairLength + 8.0f, CrosshairGap + CrosshairLength + 8.0f));
			CanvasSlot->SetSize(FVector2D(360.0f, 32.0f));
		}
	}
}

void UModelViewerCrosshairWidget::TickTargetNameAnimation(float DeltaTime)
{
	if (!ModelNameText || (!bNameTextVisible && !bNameAnimationShowing))
	{
		return;
	}

	NameAnimationElapsed += FMath::Max(DeltaTime, 0.0f);
	const float Duration = bNameAnimationShowing
		? NameFadeInDuration : NameFadeOutDuration;
	const float Alpha = FMath::Clamp(NameAnimationElapsed / FMath::Max(Duration, 0.01f), 0.0f, 1.0f);
	const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	float Opacity = 1.0f - SmoothAlpha;
	float Scale = FMath::Lerp(1.0f, 0.92f, SmoothAlpha);
	if (bNameAnimationShowing)
	{
		const float X = Alpha - 1.0f;
		const float EaseOutBack = 1.0f + 2.70158f * X * X * X + 1.70158f * X * X;
		Opacity = SmoothAlpha;
		Scale = FMath::Lerp(NameStartScale, 1.0f, EaseOutBack);
	}

	ModelNameText->SetRenderOpacity(Opacity);
	ModelNameText->SetRenderScale(FVector2D(Scale));
	if (Alpha >= 1.0f && !bNameAnimationShowing)
	{
		ModelNameText->SetVisibility(ESlateVisibility::Collapsed);
		ModelNameText->SetText(FText::GetEmpty());
		bNameTextVisible = false;
	}
}

int32 UModelViewerCrosshairWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const int32 BaseLayer = Super::NativePaint(
		Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
		InWidgetStyle, bParentEnabled);
	const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;
	const float Inner = CrosshairGap;
	const float Outer = CrosshairGap + CrosshairLength;

	const auto DrawSegment = [&](const FVector2D& Start, const FVector2D& End)
	{
		TArray<FVector2D> Segment;
		Segment.Add(Start);
		Segment.Add(End);
		FSlateDrawElement::MakeLines(
			OutDrawElements, BaseLayer + 1, AllottedGeometry.ToPaintGeometry(),
			Segment, ESlateDrawEffect::None, CrosshairColor, true, CrosshairThickness);
	};
	DrawSegment(Center + FVector2D(-Outer, 0.0f), Center + FVector2D(-Inner, 0.0f));
	DrawSegment(Center + FVector2D(Inner, 0.0f), Center + FVector2D(Outer, 0.0f));
	DrawSegment(Center + FVector2D(0.0f, -Outer), Center + FVector2D(0.0f, -Inner));
	DrawSegment(Center + FVector2D(0.0f, Inner), Center + FVector2D(0.0f, Outer));

	return BaseLayer + 1;
}
