#version 450
#pragma shader_stage(fragment)

#include "../../shaders/definition.glsl"
#include "../../shaders/jitter.glsl"
#include "../../shaders/utils.glsl"
#include "./taa_definition.glsl"
#include "./taa_utils.glsl"

layout(location = 0) in vec2 uv;
// layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gMotionVectorDepthSpecularTexture;

layout(set = 1, binding = 2) uniform ScreenDimension { vec2 resolution; }
screenDimension;

layout(set = 1, binding = 3) uniform Taa { vec2 jitter; }
uTaa;

float linearDepth(float depth) {
  return (2.0 * NEAR_Z) / (FAR_Z + NEAR_Z - depth * (FAR_Z - NEAR_Z));
}

vec2 getClosestMotionVector(vec2 jitteredUV, vec2 resolution) {
  vec2 closestOffset = vec2(0.0f, 0.0f);
  float closestDepth = FAR_Z;

  for (int y = -1; y <= 1; ++y) {
    for (int x = -1; x <= 1; ++x) {
      vec2 sampleOffset =
          vec2(float(x) / resolution.x, float(y) / resolution.y);
      vec2 sampleUV = jitteredUV + sampleOffset;
      sampleUV = saturateVec2(sampleUV);

      float NeighborhoodDepthSamp =
          texture(gMotionVectorDepthSpecularTexture, sampleUV).z;

      NeighborhoodDepthSamp = linearDepth(NeighborhoodDepthSamp);

      // #if USE_REVERSE_Z
      // if (NeighborhoodDepthSamp > closestDepth)
      // #else
      if (NeighborhoodDepthSamp < closestDepth)
      // #endif
      {
        closestDepth = NeighborhoodDepthSamp;
        closestOffset = vec2(x, y);
      }
    }
  }
  closestOffset /= resolution;

  return texture(gMotionVectorDepthSpecularTexture, jitteredUV + closestOffset)
      .xy;
}

void sampleNeighborhoods(in vec2 unjitteredUV, in vec2 resolution, out vec3 m1,
                         out vec3 m2, out vec3 minNeighbor,
                         out vec3 maxNeighbor) {
  maxNeighbor = vec3(-99999999.0f, -99999999.0f, -99999999.0f);
  minNeighbor = vec3(9999999.0f, 9999999.0f, 9999999.0f);
  m1 = vec3(0.0);
  m2 = vec3(0.0);

  for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
      vec2 neighborUv =
          saturateVec2(unjitteredUV +
                       vec2(float(x) / resolution.x, float(y) / resolution.y));

      vec3 neighborTexel = getCurrentColor(neighborUv, resolution).xyz;

#ifdef USE_TONEMAP
      neighborTexel = toneMap(neighborTexel);
#endif

      neighborTexel = rgb2YCoCgR(neighborTexel);

      minNeighbor = min(minNeighbor, neighborTexel);
      maxNeighbor = max(maxNeighbor, neighborTexel);
      m1 += neighborTexel;
      m2 += neighborTexel * neighborTexel;
    }
  }
}

void varianceClip(in vec3 m1, in vec3 m2, out vec3 aabbMin, out vec3 aabbMax) {
  const uint N = 9;
  const float VarianceClipGamma = 1.0f;

  vec3 mu = m1 / N;
  vec3 sigma = sqrt(abs(m2 / N - mu * mu));

  aabbMin = mu - VarianceClipGamma * sigma;
  aabbMax = mu + VarianceClipGamma * sigma;
}

vec3 clipAABB(vec3 aabbMin, vec3 aabbMax, vec3 prevColor) {
  // note: only clips towards aabb center (but fast!)
  vec3 p_clip = 0.5 * (aabbMax + aabbMin);
  vec3 e_clip = 0.5 * (aabbMax - aabbMin);

  vec3 v_clip = prevColor - p_clip;
  vec3 v_unit = v_clip.xyz / e_clip;
  vec3 a_unit = abs(v_unit);
  float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

  if (ma_unit > 1.0)
    return p_clip + v_clip / ma_unit;
  else
    return prevColor; // point inside aabb
}

vec3 accumulateByExponentMovingAverage(float lenMotionVector, vec3 currentColor,
                                       vec3 prevColor) {
  const float BlendWeightLowerBound = 0.03;
  const float BlendWeightUpperBound = 0.12;
  const float BlendWeightVelocityScale = 100.0 * 60.0;

  float weight = mix(BlendWeightLowerBound, BlendWeightUpperBound,
                     saturateFloat(lenMotionVector * BlendWeightVelocityScale));

  return mix(prevColor, currentColor, weight);
}

void main() {
  vec2 jitterdUV = getJitterdUV(uv, uTaa.jitter);
  vec2 unjitteredUV = getUnJitterdUV(uv, uTaa.jitter);

  vec2 motionVector =
      getClosestMotionVector(jitterdUV, screenDimension.resolution);

  float lenMotionVector = length(motionVector);

  vec3 currentColor =
      getCurrentColor(unjitteredUV, screenDimension.resolution).xyz;

#ifdef USE_TONEMAP
  currentColor = toneMap(currentColor);
#endif

  currentColor = rgb2YCoCgR(currentColor);

  vec3 prevColor =
      getPrevColor(jitterdUV - motionVector, screenDimension.resolution).xyz;

#ifdef USE_TONEMAP
  prevColor = toneMap(prevColor);
#endif

  prevColor = rgb2YCoCgR(prevColor);

  vec3 m1;
  vec3 m2;
  vec3 minNeighbor;
  vec3 maxNeighbor;
  sampleNeighborhoods(unjitteredUV, screenDimension.resolution, m1, m2,
                      minNeighbor, maxNeighbor);

  vec3 aabbMin;
  vec3 aabbMax;
  varianceClip(m1, m2, aabbMin, aabbMax);

  prevColor = clipAABB(aabbMin, aabbMax, prevColor);

  vec3 color = accumulateByExponentMovingAverage(lenMotionVector, currentColor,
                                                 prevColor);
  color = yCoCgR2RGB(color);

#ifdef USE_TONEMAP
  color = unToneMap(color);
#endif

  // outColor = vec4(color, 1.0);

  setCurrentColor(uv, screenDimension.resolution, vec4(color, 1.0));

  setPrevColor(uv, screenDimension.resolution, vec4(color, 1.0));
}