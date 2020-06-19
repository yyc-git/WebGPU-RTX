
float saturateFloat(float v) {
  if (v < 0.0) {
    return 0.0;
  }

  if (v > 1.0) {
    return 1.0;
  }

  return v;
}

vec2 saturateVec2(vec2 v) {
  return vec2(saturateFloat(v.x), saturateFloat(v.y));
}

uint convertBufferTwoDIndexToOneDIndex(uint twoDIndexX, uint twoDIndexY,
                                       uint bufferSizeX) {
  return twoDIndexY * bufferSizeX + twoDIndexX;
}

uint getPixelIndex(vec2 uv, vec2 resolution) {
  const ivec2 bufferCoord = ivec2(floor(uv * resolution));

  return convertBufferTwoDIndexToOneDIndex(bufferCoord.x, bufferCoord.y,
                                           uint(resolution.x));
}

float luminance(in vec3 color) { return dot(color, vec3(0.25f, 0.50f, 0.25f)); }