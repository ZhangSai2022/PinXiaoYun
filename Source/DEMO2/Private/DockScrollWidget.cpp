#include "DockScrollWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Engine/World.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Types/SlateConstants.h"

void UDockScrollWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!WidgetTree->RootWidget)
	{
		BuildDefaultWidgetTree();
	}
	SetIsFocusable(true);
}

void UDockScrollWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DockScrollBox)
	{
		DockScrollBox->SetOrientation(EOrientation::Orient_Horizontal);
		DockScrollBox->SetScrollBarVisibility(ESlateVisibility::Collapsed);
		DockScrollBox->SetAllowRightClickDragScrolling(true);
		DockScrollBox->SetAnimateWheelScrolling(true);
		DockScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::Always);
		DockScrollBox->SetWheelScrollMultiplier(WheelStep / FMath::Max(GetGlobalScrollAmount(), 1.0f));
	}

	RefreshItems();
}

void UDockScrollWidget::BuildDefaultWidgetTree()
{
	USizeBox* Root = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DockRoot"));
	Root->SetHeightOverride(ItemHeight * MaximumScale + LiftAtMaximumScale + 42.0f);
	WidgetTree->RootWidget = Root;

	DockScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("DockScrollBox"));
	Root->AddChild(DockScrollBox);
}

void UDockScrollWidget::InitializeDock(
	const TArray<FDockScrollItemData>& InItems,
	TSubclassOf<UDockScrollItemWidget> InItemWidgetClass,
	int32 InitialIndex)
{
	Items = InItems;
	ItemWidgetClass = InItemWidgetClass;
	SelectedIndex = Items.IsValidIndex(InitialIndex) ? InitialIndex : INDEX_NONE;
	bPendingInitialSelection = SelectedIndex != INDEX_NONE;
	RefreshItems();
}

void UDockScrollWidget::SetItems(const TArray<FDockScrollItemData>& InItems)
{
	Items = InItems;
	SelectedIndex = Items.IsValidIndex(SelectedIndex) ? SelectedIndex : INDEX_NONE;
	bPendingInitialSelection = SelectedIndex != INDEX_NONE;
	RefreshItems();
}

void UDockScrollWidget::RefreshItems()
{
	if (!DockScrollBox || !IsConstructed())
	{
		return;
	}

	DockScrollBox->ClearChildren();
	SpawnedItems.Reset();
	HoveredIndex = INDEX_NONE;

	TSubclassOf<UDockScrollItemWidget> ClassToCreate = ItemWidgetClass;
	if (!ClassToCreate)
	{
		ClassToCreate = UDockScrollItemWidget::StaticClass();
	}

	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		UDockScrollItemWidget* Item = CreateWidget<UDockScrollItemWidget>(this, ClassToCreate);
		if (!Item)
		{
			continue;
		}

		Item->InitializeItem(Items[Index], Index, this);
		SpawnedItems.Add(Item);

		const FName FrameName(*FString::Printf(TEXT("DockItemFrame_%d"), Index));
		USizeBox* ItemFrame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), FrameName);
		ItemFrame->SetWidthOverride(ItemWidth);
		ItemFrame->SetHeightOverride(ItemHeight);
		ItemFrame->AddChild(Item);
		if (UScrollBoxSlot* ItemSlot = Cast<UScrollBoxSlot>(DockScrollBox->AddChild(ItemFrame)))
		{
			ItemSlot->SetPadding(FMargin(ItemSpacing * 0.5f, 0.0f));
			ItemSlot->SetHorizontalAlignment(HAlign_Center);
			ItemSlot->SetVerticalAlignment(VAlign_Bottom);
		}
	}

	CurrentIndex = SelectedIndex;
	bPendingInitialSelection = SelectedIndex != INDEX_NONE;
}

void UDockScrollWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bPendingInitialSelection && DockScrollBox && SpawnedItems.IsValidIndex(SelectedIndex))
	{
		ScrollItemIntoCenter(SelectedIndex);
		bPendingInitialSelection = false;
	}
	UpdateMagnification(InDeltaTime);
}

void UDockScrollWidget::UpdateMagnification(float DeltaTime)
{
	if (!DockScrollBox || SpawnedItems.IsEmpty())
	{
		return;
	}

	const FGeometry ScrollGeometry = DockScrollBox->GetCachedGeometry();
	const float DockCenterX = ScrollGeometry.GetAbsolutePosition().X + ScrollGeometry.GetAbsoluteSize().X * 0.5f;
	const bool bHasHoverAnchor = SpawnedItems.IsValidIndex(HoveredIndex);
	const bool bHasSelectionAnchor = !bHasHoverAnchor && SpawnedItems.IsValidIndex(SelectedIndex);
	float MagnificationAnchorX = DockCenterX;
	if (bHasHoverAnchor)
	{
		const FGeometry HoveredGeometry = SpawnedItems[HoveredIndex]->GetCachedGeometry();
		MagnificationAnchorX = HoveredGeometry.GetAbsolutePosition().X + HoveredGeometry.GetAbsoluteSize().X * 0.5f;
	}
	else if (bHasSelectionAnchor)
	{
		const FGeometry SelectedGeometry = SpawnedItems[SelectedIndex]->GetCachedGeometry();
		MagnificationAnchorX = SelectedGeometry.GetAbsolutePosition().X + SelectedGeometry.GetAbsoluteSize().X * 0.5f;
	}

	float ClosestDistance = TNumericLimits<float>::Max();
	int32 ClosestIndex = INDEX_NONE;

	for (int32 Index = 0; Index < SpawnedItems.Num(); ++Index)
	{
		UDockScrollItemWidget* Item = SpawnedItems[Index];
		if (!IsValid(Item))
		{
			continue;
		}

		const FGeometry ItemGeometry = Item->GetCachedGeometry();
		const float ItemCenterX = ItemGeometry.GetAbsolutePosition().X + ItemGeometry.GetAbsoluteSize().X * 0.5f;
		const float Distance = FMath::Abs(ItemCenterX - MagnificationAnchorX);
		const int32 AnchorIndex = bHasHoverAnchor ? HoveredIndex : SelectedIndex;
		const int32 IndexDistance = FMath::Abs(Index - AnchorIndex);
		float Influence = 0.0f;
		if (bHasHoverAnchor && IndexDistance <= 1)
		{
			const float NormalizedDistance = Distance / FMath::Max(MagnificationRadius, 1.0f);
			const float DistanceInfluence = FMath::Exp(-NormalizedDistance * NormalizedDistance * 2.0f);
			Influence = IndexDistance == 0 ? DistanceInfluence : DistanceInfluence * 0.34f;
		}
		else if (bHasSelectionAnchor && Index == SelectedIndex)
		{
			Influence = 1.0f;
		}
		const float TargetScale = 1.0f + (MaximumScale - 1.0f) * Influence;
		const float CurrentScale = Item->GetRenderTransform().Scale.X;
		const float NewScale = FMath::FInterpTo(CurrentScale, TargetScale, DeltaTime, ScaleInterpolationSpeed);
		const float ScaleAlpha = (NewScale - 1.0f) / FMath::Max(MaximumScale - 1.0f, KINDA_SMALL_NUMBER);
		const float Opacity = Influence > KINDA_SMALL_NUMBER
			? FMath::Lerp(0.86f, 1.0f, Influence)
			: 1.0f;
		Item->SetDockVisualState(
			HoveredIndex == Index,
			SelectedIndex == Index,
			CurrentIndex == Index);
		Item->SetDockScale(NewScale, LiftAtMaximumScale * ScaleAlpha, Opacity);
		Item->TickDockVisual(DeltaTime);

		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestIndex = Index;
		}
	}

	CurrentIndex = bHasHoverAnchor ? ClosestIndex : (bHasSelectionAnchor ? SelectedIndex : INDEX_NONE);
}

void UDockScrollWidget::SetHoveredItem(int32 Index)
{
	if (Items.IsValidIndex(Index))
	{
		HoveredIndex = Index;
	}
}

void UDockScrollWidget::ClearHoveredItem(int32 Index)
{
	if (HoveredIndex == Index)
	{
		HoveredIndex = INDEX_NONE;
	}
}

FReply UDockScrollWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!DockScrollBox || Items.IsEmpty())
	{
		return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
	}

	const float Direction = InMouseEvent.GetWheelDelta() > 0.0f ? -1.0f : 1.0f;
	DockScrollBox->SetScrollOffset(DockScrollBox->GetScrollOffset() + Direction * WheelStep);
	return FReply::Handled();
}

FReply UDockScrollWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Left)
	{
		SelectPrevious();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Right)
	{
		SelectNext();
		return FReply::Handled();
	}
	if ((InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::SpaceBar) && CurrentIndex != INDEX_NONE)
	{
		SelectIndex(CurrentIndex, true);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDockScrollWidget::SelectIndex(int32 Index, bool bBroadcastEvent)
{
	if (!Items.IsValidIndex(Index))
	{
		return;
	}

	SelectedIndex = Index;
	CurrentIndex = Index;
	ScrollItemIntoCenter(Index);

	if (SpawnedItems.IsValidIndex(Index))
	{
		SpawnedItems[Index]->SetKeyboardFocus();
	}
	if (bBroadcastEvent)
	{
		OnItemSelected.Broadcast(Index, Items[Index].ItemId, Items[Index].DisplayName);
	}
}

void UDockScrollWidget::SelectNext()
{
	const int32 BaseIndex = CurrentIndex == INDEX_NONE ? 0 : CurrentIndex;
	SelectIndex(FMath::Min(BaseIndex + 1, Items.Num() - 1), true);
}

void UDockScrollWidget::SelectPrevious()
{
	const int32 BaseIndex = CurrentIndex == INDEX_NONE ? 0 : CurrentIndex;
	SelectIndex(FMath::Max(BaseIndex - 1, 0), true);
}

void UDockScrollWidget::ScrollItemIntoCenter(int32 Index)
{
	if (DockScrollBox && SpawnedItems.IsValidIndex(Index))
	{
		DockScrollBox->ScrollWidgetIntoView(SpawnedItems[Index], true, EDescendantScrollDestination::Center, 0.0f);
	}
}
