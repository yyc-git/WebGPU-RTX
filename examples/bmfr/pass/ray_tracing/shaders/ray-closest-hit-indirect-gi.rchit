#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#pragma shader_stage(closest)

#include "../../shaders/camera.glsl"
#include "../../shaders/common_data.glsl"
#include "get_hit_shading_data.glsl"
#include "shading_data.glsl"

#include "random.glsl"
#include "raycommon.glsl"

#include "light.glsl"
#include "pbr_compute.glsl"

#include "disney.glsl"

layout(location = 1) rayPayloadNV bool isShadowed;

#include "shadow_ray.glsl"

#include "ggx_direct.glsl"

#include "indirect_utils.glsl"

layout(location = 0) rayPayloadInNV hitPayload prd;

#include "indirect_ray.glsl"

#include "ggx_indirect.glsl"

layout(set = 1, binding = 0) uniform accelerationStructureNV topLevelAS;

layout(std140, set = 1, binding = 2) uniform CommonData { vec4 compressedData; }
pushC;

void main() {
  vec4 commonDataCompressedData = pushC.compressedData;
  uint lightCount = getLightCount(commonDataCompressedData);

  const float tMin = 0.1;

  HitShadingData data = getHitShadingData(gl_InstanceID, gl_PrimitiveID);

  ShadingData shading =
      buildShadingData(data.materialDiffuse, data.materialMetalness,
                       data.materialRoughness, data.materialSpecular);

  prd.hitValue += computeDirectLight(
      prd.seed, prd.evalDisneyType, tMin, lightCount, data.worldPosition,
      data.worldNormal, data.V, shading, topLevelAS);

  prd.V = data.V;
  prd.worldPosition = data.worldPosition;
  prd.worldNormal = data.worldNormal;

  // prd.shading = shading;
  prd.materialDiffuse = data.materialDiffuse;
  prd.materialMetalness = data.materialMetalness;
  prd.materialRoughness = data.materialRoughness;
  prd.materialSpecular = data.materialSpecular;
}