
vec3 computeV(vec3 cameraPosition, vec3 worldPosition){
  return normalize(cameraPosition - worldPosition);
}