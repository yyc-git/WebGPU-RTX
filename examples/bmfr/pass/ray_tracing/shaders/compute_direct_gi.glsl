
vec3 computeDirectGI(uint seed, float tMin, uint lightCount, vec3 worldPosition,
                     vec3 worldNormal, vec3 materialDiffuse,
                     vec3 materialSpecular, float shininess,

                     accelerationStructureNV topLevelAS) {
  uint lightIndexToSample = min(uint(rnd(seed) * lightCount), lightCount - 1);
  // uint lightIndexToSample = 0;

  vec3 lightDir;
  float lightIntensity;
  float lightDistance;

  getLightData(lightIndexToSample, lightIntensity, lightDistance, lightDir);

  // Diffuse
  vec3 diffuse = computeDiffuse(materialDiffuse, lightDir, worldNormal);

  float attenuation = 1.0;

  vec3 specular = vec3(0.0);

  // Tracing shadow ray only if the light is visible from the surface
  if (isLightVisibleFromTheSurface(worldNormal, lightDir)) {
    float tMax = lightDistance;
    // vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV *
    // gl_HitTNV;
    vec3 origin = vec3(worldPosition);
    vec3 rayDir = lightDir;

    bool isShadowed =
        shadowRayVisibility(topLevelAS, 1, origin, rayDir, tMin, tMax);

    if (isShadowed) {
      attenuation = 0.3;
    } else {
      specular = computeSpecular(materialSpecular, shininess,
                                 getViewDir(vec3(worldPosition)), lightDir,
                                 worldNormal);
    }
  }

  attenuation *= float(lightCount);

  return computeColor(lightIntensity, attenuation, diffuse, specular);
}