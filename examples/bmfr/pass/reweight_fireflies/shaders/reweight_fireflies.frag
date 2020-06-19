#version 450
#pragma shader_stage(fragment)

#include "../../shaders/utils.glsl"

layout(location = 0) in vec2 uv;

layout(std140, set = 0, binding = 0) buffer CurNoisyPixelBuffer {
  vec4 pixels[];
}
curNoisyPixelBuffer;

layout(std140, set = 0, binding = 1) buffer CascadeBuffers { vec4 pixels[]; }
cascadeBuffers;

layout(set = 0, binding = 2) uniform Param {
  //   float oneOverK;    // 1/kappa

  float k; // kappa
  //   float currScale;   // N/kappa / b^i_<curr>;
  float spp; // N
}
param;

layout(set = 0, binding = 3) uniform ScreenDimension { vec2 resolution; }
screenDimension;

uint _getOneCascadeBufferOffset() {
  return uint(screenDimension.resolution.x) *
         uint(screenDimension.resolution.y);
}

vec3 _getCascadeBufferColor(uint bufferIndex, uint pixelIndex) {
  return cascadeBuffers
      .pixels[_getOneCascadeBufferOffset() * bufferIndex + pixelIndex]
      .rgb;
}

void _setCascadeBufferColor(uint bufferIndex, uint pixelIndex, vec3 color) {
  cascadeBuffers
      .pixels[_getOneCascadeBufferOffset() * bufferIndex + pixelIndex] =
      vec4(color, 0.0);
}

uint _getCascadeStart() { return 1; }

uint _getCascadeBufferCount() { return 6; }

// b
uint _getCascadeBase() { return 8; }

void _splitSampleSplatting(uint pixelIndex, float luminance, vec3 color) {
  const uint cascadeStart = _getCascadeStart();
  const uint cascadeBase = _getCascadeBase();
  const uint cascadeBufferCount = _getCascadeBufferCount();

  float lowerScale = cascadeStart;
  float upperScale = lowerScale * cascadeBase;
  int baseIndex = 0;

  /* find adjacent layers in cascade for <luminance> */
  while (!(luminance < upperScale) && baseIndex < cascadeBufferCount - 2) {
    lowerScale = upperScale;
    upperScale *= cascadeBase;
    ++baseIndex;
  }

  //   // buffers are (<baseIndex>, <baseIndex>+1)
  //   CascadeSlice slice = {&cascadeBuffers[baseIndex],
  //                         &cascadeBuffers[baseIndex + 1]};

  float weightLower;
  float weightUppper;

  /* weight for lower buffer */
  if (luminance <= lowerScale)
    weightLower = 1.0f;
  else if (luminance < upperScale)
    weightLower = max(0.0f, (lowerScale / luminance - lowerScale / upperScale) /
                                (1 - lowerScale / upperScale));
  else // Inf, NaN ...
    weightLower = 0.0f;

  /* weight for higher buffer */
  if (luminance < upperScale)
    weightUppper = max(0.0f, 1 - weightLower);
  else // out of cascade range, we don't expect this to converge
    weightUppper = upperScale / luminance;

  // baseIndex = 0;
  // pixelIndex = 100;

  _setCascadeBufferColor(baseIndex, pixelIndex, color * weightLower);
  _setCascadeBufferColor(baseIndex + 1, pixelIndex, color * weightUppper);
}

// average of <r>-radius pixel block in <layer> at <coord> (only using r=0 and
// r=1)
vec3 _sampleLayer(uint bufferIndex, uint pixelIndex, const int r, float scale) {
  vec3 val = vec3(0);
  int y = -r;
  for (int i = 0; i < 3; ++i) { // WebGL requires static loops
    int x = -r;
    for (int j = 0; j < 3; ++j) { // WebGL requires static loops
      // vec4 c = texture2D(layer, (coord + vec2(x, y)) / size, 0.);
      vec3 c = _getCascadeBufferColor(
          bufferIndex,
          pixelIndex + convertBufferTwoDIndexToOneDIndex(
                           j, i, uint(screenDimension.resolution.x)));
      c *= scale;
      val += c;
      if (++x > r)
        break;
    }
    if (++y > r)
      break;
  }
  return val / float((2 * r + 1) * (2 * r + 1));
}

float _sampleReliability(uint currBufferIndex, uint pixelIndex, float currScale,
                         const int r) {
  vec3 rel = _sampleLayer(currBufferIndex, pixelIndex, r, currScale);

  if (currBufferIndex != 0) {
    // scale by N/kappa / b^i_<pref>
    rel += _sampleLayer(currBufferIndex - 1, pixelIndex, r,
                        currScale * _getCascadeBase());
  }

  if (currBufferIndex != _getCascadeBufferCount() - 1) {
    // scale by N/kappa / b^i_<next>
    rel += _sampleLayer(currBufferIndex + 1, pixelIndex, r,
                        currScale / _getCascadeBase());
  }

  // reliability is simply the luminance of the brightness-normalized layer
  return luminance(rel);
}

float _computeReliability(uint pixelIndex, float luminance, float currScale,
                          uint currBufferIndex) {

  float oneOverK = 1.0 / param.k;

  // allow re-weighting to be disabled esily for the viewer demo
  if (!(oneOverK < 1.e6)) {
    return 1.0;
  }

  /* sample counting-based reliability estimation */

  // reliability in 3x3 pixel block (see robustness)
  float globalReliability =
      _sampleReliability(currBufferIndex, pixelIndex, currScale, 1);
  // reliability of curent pixel
  float localReliability =
      _sampleReliability(currBufferIndex, pixelIndex, currScale, 0);

  float reliability = globalReliability - oneOverK;
  // check if above minimum sampling threshold
  if (reliability >= 0.)
    // then use per-pixel reliability
    reliability = localReliability - oneOverK;

  /* color-based reliability estimation */

  float colorReliability = luminance * currScale;

  // a minimum image brightness that we always consider reliable
  colorReliability = max(colorReliability, 0.05f * currScale);

  // if not interested in exact expected value estimation, can usually accept a
  // bit more variance relative to the image brightness we already have
  float optimizeForError = max(0.0f, min(1.0f, oneOverK));
  // allow up to ~<cascadeBase> more energy in one sample to lessen bias in some
  // cases
  colorReliability *= mix(mix(1., _getCascadeBase(), .6), 1., optimizeForError);

  reliability = (reliability + colorReliability) * 0.5f;

  return clamp(reliability, 0.0f, 1.0f);
}

vec3 _reweight(uint pixelIndex) {
  const uint cascadeStart = _getCascadeStart();

  float currScale = param.spp / (param.k * cascadeStart);
  // reliability in 3x3 pixel block (see robustness)
  float globalReliability;
  // reliability of curent pixel
  float localReliability;

  vec3 reweightedColor = vec3(0.0);

  for (uint i = 0; i < 6; i++) {
    reweightedColor +=
        _computeReliability(pixelIndex, luminance(reweightedColor), currScale,
                            i) *
        _getCascadeBufferColor(i, pixelIndex);

    currScale *= _getCascadeBase();
  }

  return reweightedColor;
}

void main() {
  uint pixelIndex = getPixelIndex(uv, screenDimension.resolution);
  vec4 currentColor = curNoisyPixelBuffer.pixels[pixelIndex];
  float currentColorLuminance = luminance(currentColor.rgb);

  _splitSampleSplatting(pixelIndex, currentColorLuminance, currentColor.rgb);
  vec3 reweightedColor = _reweight(pixelIndex);

  curNoisyPixelBuffer.pixels[pixelIndex] =
      vec4(reweightedColor, currentColor.a);
}