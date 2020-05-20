#version 460
#extension GL_NV_ray_tracing : require
#pragma shader_stage(miss)

#include "raycommon.glsl"

layout(location = 0) rayPayloadInNV hitPayload prd;

void main() {
  prd.hitValue = vec3(0.15);
}
