#version 450
#pragma shader_stage(fragment)

#include "../../shaders/definition.glsl"
#include "../../shaders/jitter.glsl"

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

float LinearDepth(float depth) {
  return (2.0 * NEAR_Z) / (FAR_Z + NEAR_Z - depth * (FAR_Z - NEAR_Z));
}

vec2 convertMotionVectorRangeTo0To1(vec2 motionVector) {
  return motionVector * 2.0 - 1.0;
}

void main() {
  vec2 unjitteredUV = getUnjitterdUV(uv, uTaa.jitter);

  vec3 motionVectorDepth = texture(gMotionVectorDepthShininessTexture, uv).xyz;
  vec2 motionVector = motionVectorDepth.xy;

  // outColor = vec4(convertMotionVectorRangeTo0To1(motionVector), 1.0,1.0);
  // return;


  float depth = LinearDepth(motionVectorDepth.z);

  uint currentColorPixelIndex = getPixelIndex(unjitteredUV, resolution);
  vec4 currentColor = pixelBuffer.pixels[currentColorPixelIndex];

  vec4 prevColor = historyPixelBuffer.pixels[getPixelIndex(
      uv - convertMotionVectorRangeTo0To1(motionVector),
      resolution)];
  float weight = 0.05;
  vec4 compositeColor = mix(currentColor, prevColor, weight);

  outColor = compositeColor;

  historyPixelBuffer.pixels[getPixelIndex(uv, resolution)] = outColor;
}