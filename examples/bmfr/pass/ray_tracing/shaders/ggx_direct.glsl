
vec3 computeDirectLight(uint seed,
                        // 0:both, 1:diffuse, 2:specular
                        uint evalDisneyType, float tMin, uint lightCount,
                        vec3 worldPosition, vec3 worldNormal, vec3 V,
                        ShadingData shading,

                        accelerationStructureEXT topLevelAS) {
  uint lightIndexToSample = min(uint(rnd(seed) * lightCount), lightCount - 1);
  // uint lightIndexToSample = 0;

  vec3 lightDir;
  float lightIntensity;
  float lightDistance;

  getLightData(lightIndexToSample, lightIntensity, lightDistance, lightDir);

  // // Diffuse
  // vec3 diffuse = computeDiffuse(materialDiffuse, lightDir, worldNormal);

  // float attenuation = 1.0;

  // vec3 specular = vec3(0.0);

  // Tracing shadow ray only if the light is visible from the surface
  if (isLightVisibleFromTheSurface(worldNormal, lightDir)) {

    float tMax = lightDistance;
    // vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT *
    // gl_HitTEXT;
    vec3 origin = worldPosition;
    vec3 rayDir = lightDir;

    bool isShadowed =
        shadowRayVisibility(topLevelAS, 1, origin, rayDir, tMin, tMax);

    if (isShadowed) {
      return vec3(0.0);
    }

    // if (isShadowed) {
    //   attenuation = 0.3;
    // } else {
    //   specular = computeSpecular(materialSpecularColor, shininess,
    //                              getViewDir(vec3(worldPosition)), lightDir,
    //                              worldNormal);
    // }
  }

  float pdf = 1 / float(lightCount);

  const vec3 N = worldNormal;
  // const vec3 V = -gl_WorldRayDirectionEXT;
  const vec3 L = lightDir;
  const vec3 H = normalize(V + L);

  const float NdotH = max(0.0, dot(N, H));
  const float NdotL = max(0.0, dot(L, N));
  const float HdotL = max(0.0, dot(H, L));
  const float NdotV = max(0.0, dot(N, V));

  // const float bsdfPdf = DisneyPdf(NdotH, NdotL, HdotL);

  vec3 f;
  switch (evalDisneyType) {
  case 1:
    f = evalDiffuse(NdotL, NdotV, HdotL, shading);
    break;
  case 2:
    f = evalSpecular(NdotL, NdotV, NdotH, HdotL, shading);
    break;
  case 0:
  default:
    f = eval(NdotL, NdotV, NdotH, HdotL, shading);
    break;
  }

  return lightIntensity * f / pdf;
}