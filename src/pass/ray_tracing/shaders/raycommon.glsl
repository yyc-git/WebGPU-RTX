
struct hitPayload {
  vec3 hitValue;
  uint seed;
  // 0:both, 1:diffuse, 2:specular
  uint evalDisneyType;
  vec3 V;

  ShadingData shading;

  vec3 worldNormal;
  vec3 worldPosition;
};