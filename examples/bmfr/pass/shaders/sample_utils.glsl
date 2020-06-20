
vec3 buildJitteredV(uint seed, vec3 cameraPosition, vec3 worldPosition) {
  vec3 jitteredCameraPosition =
      vec3(cameraPosition.x + rnd(seed), cameraPosition.y + rnd(seed),
           cameraPosition.z + rnd(seed));

  return computeV(jitteredCameraPosition, worldPosition);
}