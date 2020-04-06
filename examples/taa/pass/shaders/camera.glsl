layout(std140, set = 2, binding = 0) uniform Camera {
  vec4 cameraPosition;
  mat4 viewMatrix;
  mat4 projectionMatrix;
}
uCamera;

vec3 getViewDir(vec3 worldPosition) {
  return normalize(vec3(uCamera.cameraPosition) - worldPosition);
}

mat4 getViewMatrix() { return uCamera.viewMatrix; }

mat4 getProjectionMatrix() { return uCamera.projectionMatrix; }