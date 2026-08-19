# Dock Scroll Widget

`UDockScrollWidget` is a reusable horizontal product selector with a background-free Apple Dock-style hover magnification effect.

## Blueprint setup

1. Create a Widget Blueprint derived from `DockScrollWidget`.
2. Optional: add a horizontal `ScrollBox` named `DockScrollBox`. If omitted, C++ creates the scroll box layout without a background panel.
3. Create an item Widget Blueprint derived from `DockScrollItemWidget`.
4. Optional: add a `Button` named `SelectButton`, an `Image` named `ItemImage`, a `TextBlock` named `ItemName`, and a `Border` named `HighlightBorder`. If omitted, C++ creates them.
5. Set the dock's `Item Widget Class` to that item Blueprint and fill `Items`, or call `Initialize Dock` at runtime.
6. Bind `On Item Selected` to receive `Selected Index`, `Item Id`, and `Display Name`.

The item subclass receives `On Dock Item Initialized` after its image and name are assigned. It also receives `On Dock Item Focus Changed` when it enters or leaves the magnified center state.

## Interaction

- Mouse wheel: horizontal scrolling.
- Right mouse drag: inertial horizontal scrolling through the underlying `ScrollBox`.
- Click: select, center, and broadcast `On Item Selected`.
- No item is selected by default. Hovering affects only the hovered item and its two immediate neighbors.
- Hover: neighboring items ease into a magnification wave; the hovered item lifts and receives a cyan highlight.
- Left/Right: select adjacent items when the dock has keyboard focus.
- Enter/Space: activate the current item.
- Hover: hand cursor.
