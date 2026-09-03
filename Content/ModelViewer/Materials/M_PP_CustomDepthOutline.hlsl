// M_PP_CustomDepthOutline - UE 5.8 Post Process Custom node body.
// A primitive is outlined when it writes StencilValue to Custom Depth-Stencil.
// Set StencilValue to 0 to match every primitive that writes Custom Depth.
// The outline is white by default and its strength pulses with View.GameTime.
float2 ColorUV = GetDefaultSceneTextureUV(Parameters, PPI_PostProcessInput0);
float2 MaskUV = GetDefaultSceneTextureUV(Parameters, PPI_CustomStencil);
float2 Texel = GetSceneTextureBufferSize(PPI_CustomStencil).zw;
float Radius = clamp(OutlineWidth, 1.0, 4.0);
float2 StepX = float2(Texel.x * Radius, 0.0);
float2 StepY = float2(0.0, Texel.y * Radius);
float2 StepD0 = float2(Texel.x * Radius, Texel.y * Radius);
float2 StepD1 = float2(-Texel.x * Radius, Texel.y * Radius);

// These four hidden SceneTexture inputs keep the scene-texture bindings alive in
// UE's material compiler; the .ID tokens are replaced with PPI_* constants.
float SceneTextureBindingGuard = (float)SceneColorInput.ID + (float)SceneDepthInput.ID + (float)CustomDepthInput.ID + (float)CustomStencilInput.ID;

float4 SceneColor = SceneTextureLookup(ColorUV, PPI_PostProcessInput0, false);
float SceneDepth = SceneTextureLookup(MaskUV, PPI_SceneDepth, false).r;
float CenterDepth = SceneTextureLookup(MaskUV, PPI_CustomDepth, false).r;
float CenterStencil = SceneTextureLookup(MaskUV, PPI_CustomStencil, false).r;
float DepthTolerance = max(DepthBias, 0.01);
float CenterVisible = step(CenterDepth, SceneDepth + DepthTolerance);
float StencilFilter = step(0.5, StencilValue);
float CenterStencilMatch = lerp(1.0, 1.0 - step(0.5, abs(CenterStencil - StencilValue)), StencilFilter);
float CenterSelected = step(0.0001, CenterDepth) * CenterStencilMatch * CenterVisible;

float2 UV0 = ClampSceneTextureUV(MaskUV + StepX, PPI_CustomStencil);
float2 UV1 = ClampSceneTextureUV(MaskUV - StepX, PPI_CustomStencil);
float2 UV2 = ClampSceneTextureUV(MaskUV + StepY, PPI_CustomStencil);
float2 UV3 = ClampSceneTextureUV(MaskUV - StepY, PPI_CustomStencil);
float2 UV4 = ClampSceneTextureUV(MaskUV + StepD0, PPI_CustomStencil);
float2 UV5 = ClampSceneTextureUV(MaskUV + StepD1, PPI_CustomStencil);
float2 UV6 = ClampSceneTextureUV(MaskUV - StepD1, PPI_CustomStencil);
float2 UV7 = ClampSceneTextureUV(MaskUV - StepD0, PPI_CustomStencil);

float S0 = SceneTextureLookup(UV0, PPI_CustomStencil, false).r;
float S1 = SceneTextureLookup(UV1, PPI_CustomStencil, false).r;
float S2 = SceneTextureLookup(UV2, PPI_CustomStencil, false).r;
float S3 = SceneTextureLookup(UV3, PPI_CustomStencil, false).r;
float S4 = SceneTextureLookup(UV4, PPI_CustomStencil, false).r;
float S5 = SceneTextureLookup(UV5, PPI_CustomStencil, false).r;
float S6 = SceneTextureLookup(UV6, PPI_CustomStencil, false).r;
float S7 = SceneTextureLookup(UV7, PPI_CustomStencil, false).r;

float D0 = SceneTextureLookup(UV0, PPI_CustomDepth, false).r;
float D1 = SceneTextureLookup(UV1, PPI_CustomDepth, false).r;
float D2 = SceneTextureLookup(UV2, PPI_CustomDepth, false).r;
float D3 = SceneTextureLookup(UV3, PPI_CustomDepth, false).r;
float D4 = SceneTextureLookup(UV4, PPI_CustomDepth, false).r;
float D5 = SceneTextureLookup(UV5, PPI_CustomDepth, false).r;
float D6 = SceneTextureLookup(UV6, PPI_CustomDepth, false).r;
float D7 = SceneTextureLookup(UV7, PPI_CustomDepth, false).r;

float Z0 = SceneTextureLookup(UV0, PPI_SceneDepth, false).r;
float Z1 = SceneTextureLookup(UV1, PPI_SceneDepth, false).r;
float Z2 = SceneTextureLookup(UV2, PPI_SceneDepth, false).r;
float Z3 = SceneTextureLookup(UV3, PPI_SceneDepth, false).r;
float Z4 = SceneTextureLookup(UV4, PPI_SceneDepth, false).r;
float Z5 = SceneTextureLookup(UV5, PPI_SceneDepth, false).r;
float Z6 = SceneTextureLookup(UV6, PPI_SceneDepth, false).r;
float Z7 = SceneTextureLookup(UV7, PPI_SceneDepth, false).r;

float M0 = step(0.0001, D0) * step(D0, Z0 + DepthTolerance) * lerp(1.0, 1.0 - step(0.5, abs(S0 - StencilValue)), StencilFilter);
float M1 = step(0.0001, D1) * step(D1, Z1 + DepthTolerance) * lerp(1.0, 1.0 - step(0.5, abs(S1 - StencilValue)), StencilFilter);
float M2 = step(0.0001, D2) * step(D2, Z2 + DepthTolerance) * lerp(1.0, 1.0 - step(0.5, abs(S2 - StencilValue)), StencilFilter);
float M3 = step(0.0001, D3) * step(D3, Z3 + DepthTolerance) * lerp(1.0, 1.0 - step(0.5, abs(S3 - StencilValue)), StencilFilter);
float M4 = step(0.0001, D4) * step(D4, Z4 + DepthTolerance) * lerp(1.0, 1.0 - step(0.5, abs(S4 - StencilValue)), StencilFilter);
float M5 = step(0.0001, D5) * step(D5, Z5 + DepthTolerance) * lerp(1.0, 1.0 - step(0.5, abs(S5 - StencilValue)), StencilFilter);
float M6 = step(0.0001, D6) * step(D6, Z6 + DepthTolerance) * lerp(1.0, 1.0 - step(0.5, abs(S6 - StencilValue)), StencilFilter);
float M7 = step(0.0001, D7) * step(D7, Z7 + DepthTolerance) * lerp(1.0, 1.0 - step(0.5, abs(S7 - StencilValue)), StencilFilter);
float NeighborSelected = max(max(max(M0, M1), max(M2, M3)), max(max(M4, M5), max(M6, M7)));
float Edge = saturate(NeighborSelected - CenterSelected);

float3 FallbackColor = float3(1.0, 1.0, 1.0);
float3 Tint = (dot(OutlineColor.rgb, OutlineColor.rgb) > 0.0001) ? OutlineColor.rgb : FallbackColor;
float BreathPhase = View.GameTime * max(BreathSpeed, 0.01) * 6.2831853;
float Breath = 0.5 + 0.5 * sin(BreathPhase);
float BreathStrength = lerp(0.35, 1.0, Breath);
float Strength = saturate(OutlineIntensity) * BreathStrength;
float3 Result = lerp(SceneColor.rgb, Tint, Edge * Strength);
return float4(Result, SceneColor.a);
