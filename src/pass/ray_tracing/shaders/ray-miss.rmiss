#version 460
#extension GL_EXT_ray_tracing : enable
#pragma shader_stage(miss)

#include "shading_data.glsl"

#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() { prd.hitValue = vec3(0.15); }
