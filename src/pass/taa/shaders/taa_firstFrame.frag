#version 450
#pragma shader_stage(fragment)

#include "../../shaders/definition.glsl"
#include "../../shaders/jitter.glsl"
#include "../../shaders/utils.glsl"
#include "./taa_definition.glsl"
#include "./taa_utils.glsl"

layout(location = 0) in vec2 uv;
// layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler sampler0;
layout(binding = 1) uniform texture2D gMotionVectorDepthSpecularTexture;

layout(set = 1, binding = 2) uniform ScreenDimension { vec2 resolution; }
screenDimension;

layout(set = 1, binding = 3) uniform Taa { vec2 jitter; }
uTaa;

void main() {
  vec2 unjitteredUV = getUnJitterdUV(uv, uTaa.jitter);

  vec4 currentColor = getCurrentColor(unjitteredUV, screenDimension.resolution);

#ifdef USE_TONEMAP
  currentColor.xyz = toneMap(currentColor.xyz);
#endif

  currentColor.xyz = rgb2YCoCgR(currentColor.xyz);

  // outColor = currentColor;

  setPrevColor(uv, screenDimension.resolution, currentColor);
}