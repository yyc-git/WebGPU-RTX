
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

vec3 rgb2YCoCgR(vec3 rgbColor) {
  vec3 yCoCgRColor;

  yCoCgRColor.y = rgbColor.r - rgbColor.b;
  float temp = rgbColor.b + yCoCgRColor.y / 2;
  yCoCgRColor.z = rgbColor.g - temp;
  yCoCgRColor.x = temp + yCoCgRColor.z / 2;

  return yCoCgRColor;
}

vec3 yCoCgR2RGB(vec3 yCoCgRColor) {
  vec3 rgbColor;

  float temp = yCoCgRColor.x - yCoCgRColor.z / 2;
  rgbColor.g = yCoCgRColor.z + temp;
  rgbColor.b = temp - yCoCgRColor.y / 2;
  rgbColor.r = rgbColor.b + yCoCgRColor.y;

  return rgbColor;
}

float luminance(in vec3 color) {
#ifdef USE_TONEMAP
  return color.r;
#else
  return dot(color, vec3(0.25f, 0.50f, 0.25f));
#endif
}

vec3 toneMap(vec3 color) {
#ifdef USE_MIXED_TONE_MAP
  float luma = luminance(color);
  if (luma <= MIXED_TONE_MAP_LINEAR_UPPER_BOUND) {
    return color;
  } else {
    return color *
           (MIXED_TONE_MAP_LINEAR_UPPER_BOUND *
                MIXED_TONE_MAP_LINEAR_UPPER_BOUND -
            luma) /
           (luma * (2 * MIXED_TONE_MAP_LINEAR_UPPER_BOUND - 1 - luma));
  }
#else
  return color / (1 + luminance(color));
#endif
}

vec3 unToneMap(vec3 color) {
#ifdef USE_MIXED_TONE_MAP
  float luma = luminance(color);
  if (luma <= MIXED_TONE_MAP_LINEAR_UPPER_BOUND) {
    return color;
  } else {
    return color *
           (MIXED_TONE_MAP_LINEAR_UPPER_BOUND *
                MIXED_TONE_MAP_LINEAR_UPPER_BOUND -
            (2 * MIXED_TONE_MAP_LINEAR_UPPER_BOUND - 1) * luma) /
           (luma * (1 - luma));
  }
#else
  return color / (1 - luminance(color));
#endif
}
