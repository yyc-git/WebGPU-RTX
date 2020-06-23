
struct hitPayload {
  vec3 hitValue;
  uint seed;
  // 0:both, 1:diffuse, 2:specular
  uint evalDisneyType;
  vec3 V;

  // TODO use struct:
  // WebGPU Node not support struct field use struct!!!(although glsl support!)
  // ShadingData shading;
  vec3 materialDiffuse;
  float materialMetalness;
  float materialRoughness;
  float materialSpecular;

  vec3 worldNormal;
  vec3 worldPosition;
};