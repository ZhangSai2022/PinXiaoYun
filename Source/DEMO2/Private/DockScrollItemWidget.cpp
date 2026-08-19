#include "DockScrollItemWidget.h"

#include "DockScrollWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

void UDockScrollItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree->RootWidget)
	{
		BuildDefaultWidgetTree();
	}

	SetIsFocusable(true);
	SetCursor(EMouseCursor::Hand);
	if (SelectButton)
	{
		SelectButton->SetCursor(EMouseCursor::Hand);
		SelectButton->OnClicked.AddUniqueDynamic(this, &UDockScrollItemWidget::HandleClicked);
		SelectButton->OnHovered.AddUniqueDynamic(this, &UDockScrollItemWidget::HandleHovered);
		SelectButton->OnUnhovered.AddUniqueDynamic(this, &UDockScrollItemWidget::HandleUnhovered);
	}
}

void UDockScrollItemWidget::BuildDefaultWidgetTree()
{
	USizeBox* Root = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DockItemRoot"));
	Root->SetWidthOverride(176.0f);
	Root->SetHeightOverride(194.0f);
	WidgetTree->RootWidget = Root;

	HighlightBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HighlightBorder"));
	HighlightBorder->SetBrushColor(FLinearColor::Transparent);
	HighlightBorder->SetPadding(FMargin(4.0f));
	Root->AddChild(HighlightBorder);

	SelectButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SelectButton"));
	SelectButton->SetBackgroundColor(FLinearColor::Transparent);
	HighlightBorder->AddChild(SelectButton);

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DockItemContent"));
	SelectButton->AddChild(Content);

	ItemImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ItemImage"));
	ItemImage->SetDesiredSizeOverride(FVector2D(142.0f, 142.0f));
	if (UVerticalBoxSlot* ImageSlot = Content->AddChildToVerticalBox(ItemImage))
	{
		ImageSlot->SetHorizontalAlignment(HAlign_Center);
		ImageSlot->SetVerticalAlignment(VAlign_Bottom);
		ImageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	ItemName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ItemName"));
	ItemName->SetJustification(ETextJustify::Center);
	ItemName->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.95f, 0.97f, 1.0f)));
	ItemName->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f));
	ItemName->SetShadowOffset(FVector2D(0.0f, 1.0f));
	FSlateFontInfo Font = ItemName->GetFont();
	Font.Size = 18;
	ItemName->SetFont(Font);
	if (UVerticalBoxSlot* TextSlot = Content->AddChildToVerticalBox(ItemName))
	{
		TextSlot->SetHorizontalAlignment(HAlign_Fill);
		TextSlot->SetPadding(FMargin(6.0f, 7.0f, 6.0f, 8.0f));
		TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}
}

void UDockScrollItemWidget::InitializeItem(
	const FDockScrollItemData& InData,
	int32 InIndex,
	UDockScrollWidget* InOwner)
{
	ItemData = InData;
	ItemIndex = InIndex;
	OwnerDock = InOwner;

	if (ItemName)
	{
		ItemName->SetText(ItemData.DisplayName);
	}
	if (ItemImage)
	{
		ItemImage->SetBrushFromTexture(ItemData.Image, true);
		ItemImage->SetVisibility(ItemData.Image ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	SetToolTipText(ItemData.DisplayName);
	BP_OnItemInitialized(ItemData, ItemIndex);
	ApplyVisualStyle();
}

void UDockScrollItemWidget::SetDockScale(float InScale, float LiftAmount, float InOpacity)
{
	SetRenderTransformPivot(FVector2D(0.5f, 1.0f));
	SetRenderScale(FVector2D(InScale, InScale));
	SetRenderTranslation(FVector2D(0.0f, -LiftAmount));
	SetRenderOpacity(InOpacity);

	const bool bCurrent = InScale >= 1.0f + (OwnerDock ? (OwnerDock->MaximumScale - 1.0f) * 0.85f : 0.15f);
	if (bCurrent != bWasCurrent)
	{
		bWasCurrent = bCurrent;
		BP_OnFocusChanged(bCurrent, InScale);
	}
}

void UDockScrollItemWidget::SetDockVisualState(bool bInHovered, bool bInSelected, bool bInCurrent)
{
	bIsHovered = bInHovered;
	bIsSelected = bInSelected;
	bCurrentState = bInCurrent;
	TargetVisualAlpha = (bIsHovered || bIsSelected || bCurrentState) ? 1.0f : 0.0f;
}

void UDockScrollItemWidget::TickDockVisual(float DeltaTime)
{
	VisualAlpha = FMath::FInterpTo(VisualAlpha, TargetVisualAlpha, DeltaTime, 18.0f);
	ApplyVisualStyle();
}

void UDockScrollItemWidget::ApplyVisualStyle()
{
	if (HighlightBorder)
	{
		const FLinearColor TargetBorderColor = bIsSelected
			? FLinearColor(0.38f, 0.84f, 1.0f, 0.88f)
			: FLinearColor(0.55f, 0.92f, 1.0f, 0.72f);
		HighlightBorder->SetBrushColor(FMath::Lerp(FLinearColor::Transparent, TargetBorderColor, VisualAlpha));
	}
	if (SelectButton)
	{
		SelectButton->SetColorAndOpacity(FMath::Lerp(
			FLinearColor(0.78f, 0.84f, 0.88f, 1.0f),
			FLinearColor(1.0f, 1.0f, 1.0f, 1.0f),
			VisualAlpha));
	}
	if (ItemName)
	{
		const FLinearColor NormalTextColor(0.72f, 0.78f, 0.82f, 1.0f);
		const FLinearColor EmphasisTextColor = bIsSelected
			? FLinearColor(0.72f, 0.94f, 1.0f, 1.0f)
			: FLinearColor(0.96f, 0.98f, 1.0f, 1.0f);
		ItemName->SetColorAndOpacity(FSlateColor(FMath::Lerp(NormalTextColor, EmphasisTextColor, VisualAlpha)));
	}
}

void UDockScrollItemWidget::ActivateItem()
{
	HandleClicked();
}

void UDockScrollItemWidget::HandleClicked()
{
	if (OwnerDock)
	{
		OwnerDock->SelectIndex(ItemIndex, true);
	}
	OnItemClicked.Broadcast(ItemIndex, ItemData.ItemId);
}

void UDockScrollItemWidget::HandleHovered()
{
	bIsHovered = true;
	if (OwnerDock)
	{
		OwnerDock->SetHoveredItem(ItemIndex);
	}
	ApplyVisualStyle();
}

void UDockScrollItemWidget::HandleUnhovered()
{
	bIsHovered = false;
	if (OwnerDock)
	{
		OwnerDock->ClearHoveredItem(ItemIndex);
	}
	ApplyVisualStyle();
}

void UDockScrollItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	HandleHovered();
}

void UDockScrollItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	HandleUnhovered();
}

FReply UDockScrollItemWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		ActivateItem();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
