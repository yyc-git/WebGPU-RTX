#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#pragma shader_stage(closest)

#include "common_data.glsl"
#include "get_hit_shading_data.glsl"

#include "random.glsl"
#include "raycommon.glsl"

#include "../../shaders/camera.glsl"
#include "light.glsl"
#include "phong_compute.glsl"

layout(location = 1) rayPayloadNV bool isShadowed;

#include "shadow_ray.glsl"

#include "compute_direct_gi.glsl"
#include "compute_indirect_gi.glsl"

layout(location = 0) rayPayloadInNV hitPayload prd;

layout(set = 1, binding = 0) uniform accelerationStructureNV topLevelAS;

layout(std140, set = 1, binding = 2) uniform CommonData { vec4 compressedData; }
pushC;

void main() {
  vec4 commonDataCompressedData = pushC.compressedData;
  uint lightCount = getLightCount(commonDataCompressedData);

  float tMin = 0.1;

  HitShadingData data = getHitShadingData(gl_InstanceID, gl_PrimitiveID);

  prd.hitValue = computeIndirectGI(
      prd.seed, tMin, lightCount, data.worldPosition, data.worldNormal,
      data.materialDiffuse, data.materialSpecular, data.shininess, topLevelAS);
}