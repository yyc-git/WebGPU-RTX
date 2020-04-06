#version 450
#pragma shader_stage(vertex)

#include "../../shaders/camera.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out float vDepth;

layout(std140, set = 0, binding = 0) uniform Model {
  mat3 normalMatrix;
  mat4 modelMatrix;
}
uModel;

void main() {
  vec4 worldPosition = uModel.modelMatrix * vec4(position, 1.0);
  vPosition = vec3(worldPosition);
  vNormal = uModel.normalMatrix * normal;

  // gl_Position = uCamera.projectionMatrix * uCamera.viewMatrix *
  // worldPosition;
  gl_Position = getProjectionMatrix() * getViewMatrix() * worldPosition;

  vDepth = gl_Position.z / gl_Position.w;
}