
layout(std140, set = 1, binding = 0) buffer PixelBuffer { vec4 pixels[]; }
pixelBuffer;
layout(std140, set = 1, binding = 1) buffer HistoryPixelBuffer {
  vec4 pixels[];
}
historyPixelBuffer;

uint getPixelIndex(vec2 jitteredUV, vec2 resolution) {
  const ivec2 bufferCoord = ivec2(floor(jitteredUV * resolution));

  return bufferCoord.y * uint(resolution.x) + bufferCoord.x;
}

vec4 getCurrentColor(vec2 unjitteredUV, vec2 resolution) {
  uint currentColorPixelIndex = getPixelIndex(unjitteredUV, resolution);

  return pixelBuffer.pixels[currentColorPixelIndex];
}

vec4 getPrevColor(vec2 jitteredUV, vec2 resolution) {
  return historyPixelBuffer.pixels[getPixelIndex(jitteredUV, resolution)];
}

void setPrevColor(vec2 jitteredUV, vec2 resolution, vec4 prevColor) {
  historyPixelBuffer.pixels[getPixelIndex(jitteredUV, resolution)] = prevColor;
}