
struct DirectionLight {
  // include intentity
  vec4 compressedData;
  vec4 position;
};

layout(std140, set = 3, binding = 0) buffer DirectionLights {
  DirectionLight l[];
}
lights;

void getLightData(in uint lightIndex, out float lightIntensity,
                  out float lightDistance, out vec3 lightDir) {
  DirectionLight data = lights.l[lightIndex];

  lightIntensity = data.compressedData.x;
  lightDistance = 100000.0;
  lightDir = normalize(vec3(data.position) - vec3(0.0));
}

bool isLightVisibleFromTheSurface(vec3 worldNormal, vec3 lightDir) {
  return dot(worldNormal, lightDir) > 0;
}