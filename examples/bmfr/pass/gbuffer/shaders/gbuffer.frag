#version 450
#pragma shader_stage(fragment)

#include "../../shaders/phong_material.glsl"

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec4 vClipPosition;
layout(location = 3) in vec4 vLastClipPosition;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gDiffuse;
layout(location = 3) out vec4 gMotionVectorDepthShininess;

void main() {
  gPosition = vec4(vPosition, 0.0);
  gNormal = normalize(vNormal);
  gDiffuse = getMaterialDiffuse();

  gMotionVectorDepthShininess =
      vec4(((vClipPosition.xy / vClipPosition.w) -
            (vLastClipPosition.xy / vLastClipPosition.w)),
           gl_FragCoord.z, getMaterialShininess());
}