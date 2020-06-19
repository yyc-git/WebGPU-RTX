
float getAlbedoMinLimit() { return 0.01; }

vec3 demodulateAlbedo(vec3 currentColor, vec3 diffuse) {
  float minLimit = getAlbedoMinLimit();

  return vec3(
      diffuse.x < minLimit ? currentColor.x : currentColor.x / diffuse.x,
      diffuse.y < minLimit ? currentColor.y : currentColor.y / diffuse.y,
      diffuse.z < minLimit ? currentColor.z : currentColor.z / diffuse.z);
}

vec3 modulateAlbedo(vec3 currentColor, vec3 diffuse) {
  float minLimit = getAlbedoMinLimit();

  return vec3(
      diffuse.x < minLimit ? currentColor.x : currentColor.x * diffuse.x,
      diffuse.y < minLimit ? currentColor.y : currentColor.y * diffuse.y,
      diffuse.z < minLimit ? currentColor.z : currentColor.z * diffuse.z);
}
