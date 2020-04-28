#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#pragma shader_stage(fragment)

#define SECOND_BLEND_ALPHA 0.1f

#include "../../../ray_tracing/shaders/common_data.glsl"
#include "../../../shaders/utils.glsl"
#include "../../shaders/utils.glsl"

layout(location = 0) in vec2 uv;

layout(std140, set = 0, binding = 0) buffer CurNoisyPixelBuffer {
  vec4 pixels[];
}
curNoisyPixelBuffer;
layout(std140, set = 0, binding = 1) buffer PccumulatedPrevFramePixelBuffer {
  vec4 pixels[];
}
accumulatedPrevFramePixelBuffer;

layout(scalar, set = 0, binding = 2) buffer AcceptBoolBuffer {
  uint acceptBools[];
}
acceptBoolBuffer;

layout(scalar, set = 0, binding = 3) buffer PrevFramePixelIndicesBuffer {
  vec2 prevFramePixelIndices[];
}
prevFramePixelIndicesBuffer;

layout(set = 0, binding = 4) uniform ScreenDimension { vec2 resolution; }
screenDimension;

layout(std140, set = 0, binding = 5) uniform CommonData { vec4 compressedData; }
pushC;

uint convertPixelIndicesToPixelIndex(ivec2 pixelIndices, vec2 resolution) {
  return convertBufferTwoDIndexToOneDIndex(pixelIndices.x, pixelIndices.y,
                                           uint(resolution.x));
}

// TODO refactor: extract first frame shader
void main() {
  vec4 commonDataCompressedData = pushC.compressedData;
  uint frame = getFrame(commonDataCompressedData);

  uint pixelIndex = getPixelIndex(uv, screenDimension.resolution);

  const vec4 filterData = curNoisyPixelBuffer.pixels[pixelIndex];
  const vec3 filteredColor = filterData.xyz;
  const float cusSpp = filterData.w;

  vec3 prevColor = vec3(0.0, 0.0, 0.0);
  float blendAlpha = 1.0;
  const uint accept = acceptBoolBuffer.acceptBools[pixelIndex];

  if (frame > 0) {
    // If any prev frame sample is accepted
    if (accept > 0) {

      // Bilinear sampling

      vec2 prevFramePixelIndicesFloat =
          prevFramePixelIndicesBuffer.prevFramePixelIndices[pixelIndex];

      ivec2 prevFramePixelIndicesInt = ivec2(prevFramePixelIndicesFloat);

      vec2 prevPixelFract =
          prevFramePixelIndicesFloat - vec2(prevFramePixelIndicesInt);

      vec2 oneMinusPrevPixelFract = 1.0 - prevPixelFract;

      float totalWeight = 0.0;

      // Accept tells if the sample is acceptable based on world position and
      // normal
      if ((accept & 0x01) != 0) {
        float weight = oneMinusPrevPixelFract.x * oneMinusPrevPixelFract.y;
        totalWeight += weight;
        prevColor +=
            weight *
            accumulatedPrevFramePixelBuffer
                .pixels[convertPixelIndicesToPixelIndex(
                    prevFramePixelIndicesInt, screenDimension.resolution)]
                .xyz;
      }

      if ((accept & 0x02) != 0) {
        float weight = prevPixelFract.x * oneMinusPrevPixelFract.y;
        totalWeight += weight;
        prevColor += weight * accumulatedPrevFramePixelBuffer
                                  .pixels[convertPixelIndicesToPixelIndex(
                                      prevFramePixelIndicesInt + ivec2(1, 0),
                                      screenDimension.resolution)]
                                  .xyz;
      }

      if ((accept & 0x04) != 0) {
        float weight = oneMinusPrevPixelFract.x * prevPixelFract.y;
        totalWeight += weight;
        prevColor += weight * accumulatedPrevFramePixelBuffer
                                  .pixels[convertPixelIndicesToPixelIndex(
                                      prevFramePixelIndicesInt + ivec2(0, 1),
                                      screenDimension.resolution)]
                                  .xyz;
      }

      if ((accept & 0x08) != 0) {
        float weight = prevPixelFract.x * prevPixelFract.y;
        totalWeight += weight;
        prevColor += weight * accumulatedPrevFramePixelBuffer
                                  .pixels[convertPixelIndicesToPixelIndex(
                                      prevFramePixelIndicesInt + ivec2(1, 1),
                                      screenDimension.resolution)]
                                  .xyz;
      }

      if (totalWeight > 0.0) {
        // Blend_alpha is dymically decided so that the result is average
        // of all samples until the cap defined by SECOND_BLEND_ALPHA is reached
        blendAlpha = 1.0 / cusSpp;
        blendAlpha = max(blendAlpha, SECOND_BLEND_ALPHA);

        prevColor /= totalWeight;
      }
    }
  }

  vec3 accumulatedColor = mix(prevColor, filteredColor, blendAlpha);
  curNoisyPixelBuffer.pixels[pixelIndex] = vec4(accumulatedColor, 1.0);

  accumulatedPrevFramePixelBuffer.pixels[pixelIndex] =
      vec4(accumulatedColor, 1.0);
}