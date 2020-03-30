#version 460
#extension GL_NV_ray_tracing : require
#pragma shader_stage(closest)

layout(location = 0) rayPayloadInNV vec3 hitValue;

hitAttributeNV vec3 attribs;

void main() {
  hitValue = vec3(0.0, 1.0, 0.0);
}
