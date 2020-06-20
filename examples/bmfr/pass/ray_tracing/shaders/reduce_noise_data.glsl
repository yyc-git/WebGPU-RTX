
uint getIndirectLightSpecularSampleCount(vec4 compressedData) {
  return uint(compressedData.x);
}