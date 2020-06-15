
#define PI 3.141592653589793
#define TWO_PI 6.283185307179586
#define INV_PI 0.3183098861837907

float _sqr(float f) { return f * f; }

float _gtr1(float NdotH, float a) {
  if (a >= 1.0)
    return INV_PI;
  float a2 = a * a;
  float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
  return (a2 - 1.0) / (PI * log(a2) * t);
}

float _gtr2(float NdotH, float a) {
  float a2 = a * a;
  float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
  return a2 / (PI * t * t);
}

float _smithGGX_G(float NdotV, float a) {
  float a2 = a * a;
  float b = NdotV * NdotV;
  return 1.0 / (NdotV + sqrt(a2 + b - a2 * b));
}

float _schlickFresnelReflectance(float u) {
  float m = clamp(1.0 - u, 0.0, 1.0);
  float m2 = m * m;
  return m2 * m2 * m;
}

float computeDiffusePdf(in const float NdotL) { return NdotL * (1.0 / PI); }

float computeSpecularPdf(in const float NdotH, in const float HdotL,
                          in ShadingData shading) {
  const float r_pdf = _gtr2(NdotH, shading.roughness) * NdotH / (4.0 * HdotL);
  const float c_pdf = _gtr1(NdotH, 0.001) * NdotH / (4.0 * HdotL);

  return c_pdf * 0.001 + r_pdf;
}

vec3 evalDiffuse(in float NdotL, in const float NdotV, in const float HdotL,
                 in ShadingData shading) {
  if (NdotL <= 0.0 || NdotV <= 0.0)
    return vec3(0);

  const vec3 cd_lin = shading.baseColor;

  // Diffuse fresnel - go from 1 at normal incidence to 0.5 at grazing
  // and mix in diffuse retro-reflection based on roughness
  const float f_wo = _schlickFresnelReflectance(NdotV);
  const float f_wi = _schlickFresnelReflectance(NdotL);

  const float fd90 = 0.5 + 2.0 * HdotL * HdotL * shading.roughness;
  const float fd = mix(1.0, fd90, f_wo) * mix(1.0, fd90, f_wi);

  const vec3 f = ((1.0 / PI) * fd * cd_lin) * (1.0 - shading.metallic);

  return f * NdotL;
}

vec3 evalSpecular(in float NdotL, in const float NdotV, in const float NdotH,
                  in const float HdotL, in ShadingData shading) {
  if (NdotL <= 0.0 || NdotV <= 0.0)
    return vec3(0);

  const vec3 cd_lin = shading.baseColor;

  const vec3 c_spec0 =
      mix(shading.specular * vec3(0.3), cd_lin, shading.metallic);

  const float ds = _gtr2(NdotH, shading.roughness);
  const float fh = _schlickFresnelReflectance(HdotL);
  const vec3 fs = mix(c_spec0, vec3(1), fh);

  float gs = 0.0;
  const float ro2 = _sqr(shading.roughness * 0.5 + 0.5);
  gs = _smithGGX_G(NdotV, ro2);
  gs *= _smithGGX_G(NdotL, ro2);

  // clearcoat (ior = 1.5 -> F0 = 0.04)
  const float dr = _gtr1(NdotH, 0.04);
  const float fr = mix(0.04, 1.0, fh);
  const float gr = _smithGGX_G(NdotV, 0.25) * _smithGGX_G(NdotL, 0.25);

  const vec3 f = gs * fs * ds + 0.001 * gr * fr * dr;

  return f * NdotL;
}

vec3 eval(in float NdotL, in const float NdotV, in const float NdotH,
          in const float HdotL, in ShadingData shading) {
  return evalDiffuse(NdotL, NdotV, HdotL, shading) +
         evalSpecular(NdotL, NdotV, NdotH, HdotL, shading);
}