
float _computeSpecularLobeProb(ShadingData shading) {

  const vec3 cd_lin = shading.baseColor;
  const float cd_lum = dot(cd_lin, vec3(0.3, 0.6, 0.1));
  const vec3 c_spec0 =
      mix(shading.specular * vec3(0.3), cd_lin, shading.metallic);
  const float cs_lum = dot(c_spec0, vec3(0.3, 0.6, 0.1));

  return cs_lum / (cs_lum + (1.0 - shading.metallic) * cd_lum);
}

vec3 _getGGXMicrofacet(float r1, float r2, vec3 worldNormal, vec3 V,
                       ShadingData shading) {
  // float r1 = rnd(seed);
  // float r2 = rnd(seed);

  // Get an orthonormal basis from the normal
  vec3 N = worldNormal;
  vec3 B = getPerpendicularVector(worldNormal);
  vec3 T = cross(B, worldNormal);

  const float a = shading.roughness;
  const float cosTheta = sqrt((1.0 - r2) / (1.0 + (a * a - 1.0) * r2));
  const float sinTheta = sqrt(max(0.0, 1.0 - (cosTheta * cosTheta)));
  const float phi = r1 * TWO_PI;
  vec3 H = normalize(vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta));
  H = H.x * T + H.y * B + H.z * N;
  // H = dot(H, V) <= 0.0 ? H * -1.0 : H;

  return H;
}

vec3 _computeSpecular(float r1, float r2, float tMin, vec3 worldPosition,
                      vec3 worldNormal, vec3 V, ShadingData shading,
                      float specularLobeProb,
                      accelerationStructureNV topLevelAS) {
  const vec3 H = _getGGXMicrofacet(r1, r2, worldNormal, V, shading);

  const vec3 L = reflect(-V, H);

  prd.evalDisneyType = 0;
  vec3 bounceColor = shootIndirectRay(topLevelAS, worldPosition, L, tMin);

  const vec3 N = worldNormal;

  const float NdotH = max(0.0, dot(N, H));
  const float NdotL = max(0.0, dot(L, N));
  const float HdotL = max(0.0, dot(H, L));
  const float NdotV = max(0.0, dot(N, V));

  return bounceColor * evalSpecular(NdotL, NdotV, NdotH, HdotL, shading) /
         (computeSpecularPdf(NdotH, HdotL, shading) * specularLobeProb);
}

vec3 _computeDiffuse(float r1, float r2, float tMin, vec3 worldPosition,
                     vec3 worldNormal, vec3 V, ShadingData shading,
                     float specularLobeProb,
                     accelerationStructureNV topLevelAS) {
  vec3 bounceDir = getCosHemisphereSample(r1, r2, worldNormal);

  prd.evalDisneyType = 1;
  vec3 bounceColor =
      shootIndirectRay(topLevelAS, worldPosition, bounceDir, tMin);

  const vec3 L = bounceDir;
  const vec3 N = worldNormal;
  const vec3 H = normalize(V + L);

  const float NdotL = max(0.0, dot(L, N));
  const float HdotL = max(0.0, dot(H, L));
  const float NdotV = max(0.0, dot(N, V));

  return bounceColor * evalDiffuse(NdotL, NdotV, HdotL, shading) /
         (computeDiffusePdf(NdotL) * (1.0 - specularLobeProb));
}

vec3 computeIndirectLight(uint seed, float tMin,

                          vec3 V, vec3 worldPosition, vec3 worldNormal,
                          ShadingData shading,
                          accelerationStructureNV topLevelAS) {

  float specularLobeProb = _computeSpecularLobeProb(shading);
  bool chooseSpecular = (rnd(seed) < specularLobeProb);

  const uint indirectLightSpecularSampleCount = 30;
  const uint indirectLightDiffuseSampleCount = 30;

  float sampleRandomArr[indirectLightSpecularSampleCount];

  for (uint i = 0; i < indirectLightSpecularSampleCount; i++) {
    sampleRandomArr[i] =
        float(i + 0.1) / (indirectLightSpecularSampleCount + 0.1);
  }

  if (chooseSpecular) {
    vec3 indirectSpecularColor = vec3(0.0);

    for (uint ss1 = 0; ss1 < indirectLightSpecularSampleCount; ++ss1) {
      indirectSpecularColor += _computeSpecular(
          rnd(seed), sampleRandomArr[ss1 % indirectLightSpecularSampleCount],
          tMin, worldPosition, worldNormal, V, shading, specularLobeProb,
          topLevelAS);
    }

    indirectSpecularColor /= indirectLightSpecularSampleCount;

    return indirectSpecularColor;
  }

  vec3 indirectDiffuseColor = vec3(0.0);

  for (uint ss2 = 0; ss2 < indirectLightDiffuseSampleCount; ++ss2) {
    indirectDiffuseColor +=
        _computeDiffuse(rnd(seed), rnd(seed), tMin, worldPosition, worldNormal,
                        V, shading, specularLobeProb, topLevelAS);
  }

  indirectDiffuseColor /= indirectLightDiffuseSampleCount;

  return indirectDiffuseColor;
}