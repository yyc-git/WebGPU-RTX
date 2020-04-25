layout(std140, set = 3, binding = 0) uniform DirectionLight {
  // include intentity
  vec4 compressedData;
  vec4 position;
}
uDirectionLight;

void getLightData(out float lightIntensity, out float lightDistance,
                  out vec3 lightDir) {
  lightIntensity = uDirectionLight.compressedData.x;
  lightDistance = 100000.0;
  lightDir = normalize(vec3(uDirectionLight.position) - vec3(0.0));
}

bool isLightVisibleFromTheSurface(vec3 worldNormal, vec3 lightDir) {
  return dot(worldNormal, lightDir) > 0;
}