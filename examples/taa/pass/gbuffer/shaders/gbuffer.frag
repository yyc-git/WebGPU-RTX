#version 450
#pragma shader_stage(fragment)

#include "../../shaders/phong_material.glsl"

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in float vDepth;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gDiffuse;
layout(location = 3) out vec3 gSpecular;
layout(location = 4) out float gShininess;
layout(location = 5) out float gDepth;

void main() {
  gPosition = vec4(vPosition, 1.0);
  gNormal = normalize(vNormal);
  // gDiffuse = vec3(uPhongMaterial.diffuse);
  // gSpecularShininess =
  //     vec4(vec3(uPhongMaterial.specular), uPhongMaterial.compressedData.x);
  gDiffuse = getMaterialDiffuse();
  gSpecular = getMaterialSpecular();
  gShininess = getMaterialShininess();
  gDepth = vDepth;
}