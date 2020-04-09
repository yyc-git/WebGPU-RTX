#version 460
#extension GL_NV_ray_tracing : require
#pragma shader_stage(miss)

layout(location = 1) rayPayloadInNV bool isShadowed;

void main() { isShadowed = false; }