# Model Viewer Template

`AModelViewerPawn` is a reusable orbit camera for product and equipment model pages.

## Setup

1. Create a Blueprint child of `ModelViewerPawn` named `BP_ModelViewerPawn`.
2. Place the pawn at the center of the model group. The pawn location is the orbit pivot.
3. Keep the model meshes in separate actors or components so they can be focused individually.
4. Set the level GameMode default pawn to `BP_ModelViewerPawn`, or place the pawn in the level and enable Auto Possess Player 0.

## Interaction

- Left mouse drag: orbit around the pivot.
- Mouse wheel: zoom with a smooth spring-arm transition.
- Middle mouse drag: pan the pivot.
- `R`: restore the initial pivot, rotation and zoom.

## Smooth component focus

Call `FocusOnActor(TargetActor, KeepCurrentYaw)` from a menu button or a model hit result. For a single mesh component, call `FocusOnPrimitive(TargetComponent, KeepCurrentYaw)`. The pawn calculates the target bounds, chooses a fitting camera distance from the field of view, and interpolates the pivot and spring arm instead of snapping.

For a custom camera point, call `FocusOnLocation(Location, Radius, KeepCurrentYaw)`. This is useful for doors, control panels, motors, shelves, and other sub-assemblies.

Each model actor must have query collision enabled if it is selected by a cursor line trace. A typical widget flow is: get the selected actor or primitive component from the hit result, then call the matching focus function on the viewer pawn.

## Door base Blueprint

Create a Blueprint child of `ModelViewerDoorBase` (for example `BP_Door_Insulated`). The base actor already contains `DoorRoot`, `ModelRoot`, and `FocusTargetRoot` scene components. Add the door mesh parts under `ModelRoot`, and add one `ModelViewerFocusTargetComponent` under `FocusTargetRoot` for every module.

The side menu can call `FocusModule(ViewerPawn, TargetId, KeepCurrentYaw)` on the door Blueprint. `FindFocusTarget` and `GetFocusTargetIds` are available when a menu needs to build itself dynamically. The door Blueprint remains movable and reusable because all focus positions are stored relative to its components.

## Blueprint API

Both `ModelViewerPawn` and `FlyingPawn` are explicitly Blueprintable and BlueprintType classes. Camera, spring arm, root component, tuning properties, and read-only runtime state are exposed to Blueprint.

UMG and derived Blueprints can call `AddOrbitInput`, `AddPanInput`, `AddZoomInput`, `SetOrbitDragging`, `SetPanDragging`, `SetViewAngles`, `SetZoomDistance`, `SetPivotLocation`, `ResetView`, `SetAutoRotate`, and all focus functions.

The Pawn binds `MouseScrollUp` and `MouseScrollDown` directly, so mouse zoom does not depend on legacy axis mappings. Configure `DefaultZoomDistance`, `MinZoomDistance`, `MaxZoomDistance`, `ZoomStep`, and `ZoomInterpSpeed` on the Blueprint defaults. Configure mouse rotation with `OrbitDragSensitivity`, keyboard rotation with `KeyboardYawSpeed` and `KeyboardPitchSpeed`, and rotation smoothing with `RotationInterpSpeed`.

If a full-screen UMG widget handles the mouse wheel itself, override the widget's `OnMouseWheel` event and call `ZoomInOneStep` when Wheel Delta is positive or `ZoomOutOneStep` when it is negative, then return Handled.

## Door module focus targets

Add `ModelViewerFocusTargetComponent` to the door Blueprint once for each module, such as `DoorLeaf`, `Window`, `Motor`, or `ControlPanel`. Set a unique `TargetId`, a local `FocusOffset`, and a `FocusRadius`. The component follows the door automatically when the door is moved or rotated.

For visual camera authoring, add a `ModelViewerFocusPointChildComponent` to the door Actor Blueprint. Its component location is the focus pivot, its component rotation is the authored orbit rotation, `FocusDistance` drives the spawned SpringArm length, and `FocusFOV` drives the spawned Camera. The component exposes `TargetId`, distance, FOV, and FOV override settings directly on the parent Blueprint while its child focus actor provides the visible SpringArm and camera preview.

`FocusModule` checks attached focus-point actors first, then falls back to legacy `ModelViewerFocusTargetComponent` markers. Existing UMG button calls therefore work with both authoring systems.

## Door module selection and presentation

Use `SelectModule(ViewerPawn, TargetId, KeepCurrentYaw)` when a menu click should focus the camera and also update the model presentation. Keep `FocusModule` for camera-only actions.

Add one `ModuleDefinitions` entry for every selectable part. Set its `TargetId` to the same value used by the focus point. Add that value to each mesh component's `Component Tags`; when a different tag is required, set `ComponentTag` on the definition. Multiple mesh components may share the same tag and are treated as one module.

`OtherModulesEffect` controls non-selected modules:

- `Keep Normal` leaves them unchanged.
- `Hidden` hides them until the selection changes or `ClearModuleSelection` is called.
- `Material Parameter` writes `OtherModuleMaterialValue` to `ModuleMaterialParameterName`. The default parameter is `ViewerOpacity`; the model's master material must expose this scalar. True transparency also requires a translucent-compatible material blend mode.

Assign `SelectedOverlayMaterial` for a direct selected overlay. `bUseCustomDepthHighlight` also enables Custom Depth and writes the module's `HighlightStencilValue`; a post-process material and the project Custom Depth-Stencil setting are required for an outline.

Override the Blueprint events `On Module Selected` and `On Module Deselected` in each door Blueprint to play or reverse a Timeline, skeletal animation, montage, or sequence. Both events receive the `TargetId` and all primitive components tagged as that module. Re-selecting the current module fires `On Module Selected` again so its animation can be replayed.

A UMG button only needs to call `SelectModule` with its focus-point `TargetId`. `GetModuleIds`, `GetModuleComponents`, `GetSelectedModuleId`, `ClearModuleSelection`, and the assignable `OnModuleSelectionChanged` delegate are also available to Blueprint.

In the editor viewport, selecting a focus target now draws the focus sphere, the camera position, a camera-to-target line, and a wireframe view cone. Enable `bUseCustomFocusDistance` and tune `FocusDistance` when a module needs a fixed camera distance. `PreviewFOV` and `PreviewAspectRatio` control the editor cone only. `FocusPitch` and `FocusYawOffset` preview the custom angle when `bUseCustomViewAngles` is enabled.

Each side-menu button can call `FocusOnTarget(TargetComponent, KeepCurrentYaw)`, or call `FocusOnTargetById(DoorActor, TargetId, KeepCurrentYaw)`. The Pawn converts the marker's local transform to world space at call time, then smoothly moves the camera pivot and zoom. No hard-coded door world position is used.
