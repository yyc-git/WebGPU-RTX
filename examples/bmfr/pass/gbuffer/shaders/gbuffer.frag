#version 450
#pragma shader_stage(fragment)

#include "../../shaders/pbr_material.glsl"

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec4 vClipPosition;
layout(location = 3) in vec4 vLastClipPosition;

layout(location = 0) out vec4 gPositionRoughness;
layout(location = 1) out vec4 gNormalMetalness;
layout(location = 2) out vec4 gDiffusePositionW;
layout(location = 3) out vec4 gMotionVectorDepthSpecular;

void main() {
  gPositionRoughness = vec4(vPosition, getMaterialRoughness());
  gNormalMetalness = vec4(normalize(vNormal), getMaterialMetalness());
  gDiffusePositionW = vec4(getMaterialDiffuse(), 0.0);

  gMotionVectorDepthSpecular =
      vec4(((vClipPosition.xy / vClipPosition.w) -
            (vLastClipPosition.xy / vLastClipPosition.w)),
           gl_FragCoord.z, getMaterialSpecular());
}