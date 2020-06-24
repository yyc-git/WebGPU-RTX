#version 450
#pragma shader_stage(fragment)

#include "../../shaders/utils.glsl"

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 0) buffer PixelBuffer { vec4 pixels[]; }
pixelBuffer;

layout(std140, set = 0, binding = 1) buffer AccumulationPixelBuffer {
  vec4 pixels[];
}
accumulationPixelBuffer;

layout(std140, set = 0, binding = 2) uniform AccumulationCommonData {
  float accumFrameCount;
}
accumulationCommonData;

layout(set = 0, binding = 3) uniform ScreenDimension { vec2 resolution; }
screenDimension;

void main() {
  uint accumFrameCount = uint(accumulationCommonData.accumFrameCount);
  uint pixelIndex = getPixelIndex(uv, screenDimension.resolution);

  vec4 accumulationColor = accumulationPixelBuffer.pixels[pixelIndex] +
                           pixelBuffer.pixels[pixelIndex];

  accumulationPixelBuffer.pixels[pixelIndex] = accumulationColor;

  vec4 finalColor = accumulationColor / accumFrameCount;

  pixelBuffer.pixels[pixelIndex] = finalColor;

  outColor = finalColor;
}