#version 460
#extension GL_NV_ray_tracing : require
#pragma shader_stage(miss)

#include "isShadowedDefine.glsl"

void main() { isShadowed = false; }
