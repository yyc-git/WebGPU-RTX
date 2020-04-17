#version 450
#pragma shader_stage(fragment)

#include "../../shaders/definition.glsl"

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gMotionVectorDepthShininessTexture;

layout(std140, set = 1, binding = 0) buffer PixelBuffer { vec4 pixels[]; }
pixelBuffer;
layout(std140, set = 1, binding = 1) buffer HistoryPixelBuffer {
  vec4 pixels[];
}
historyPixelBuffer;

layout(set = 1, binding = 2) uniform ScreenDimension { vec2 resolution; };

layout(set = 1, binding = 3) uniform Taa { vec2 jitter; }
uTaa;

uint getPixelIndex(vec2 uv, vec2 resolution) {
  const ivec2 bufferCoord = ivec2(floor(uv * resolution));

  return bufferCoord.y * uint(resolution.x) + bufferCoord.x;
}

void main() {
  vec2 unjitteredUV = uv - uTaa.jitter;

  uint currentColorPixelIndex = getPixelIndex(unjitteredUV, resolution);
  vec4 currentColor = pixelBuffer.pixels[currentColorPixelIndex];

  historyPixelBuffer.pixels[currentColorPixelIndex] = currentColor;

  outColor = currentColor;
}