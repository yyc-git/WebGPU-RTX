

vec3 computeIndirectGI(uint seed, float tMin, uint lightCount,
                       vec3 worldPosition, vec3 worldNormal,
                       vec3 materialDiffuse, vec3 materialSpecular,
                       float shininess, accelerationStructureNV topLevelAS) {
  return computeDirectGI(seed, tMin, lightCount, worldPosition, worldNormal,
                         materialDiffuse, materialSpecular, shininess,
                         topLevelAS);
}