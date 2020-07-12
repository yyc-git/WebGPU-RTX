layout(std140, set = 1, binding = 0) uniform PBRMaterial {
  vec4 diffuse;
  // include metalness, roughness, specular
  vec4 compressedData;
}
uPBRMaterial;

vec3 getMaterialDiffuse() { return vec3(uPBRMaterial.diffuse); }

float getMaterialMetalness() { return uPBRMaterial.compressedData.x; }

float getMaterialRoughness() { return uPBRMaterial.compressedData.y; }

float getMaterialSpecular() { return uPBRMaterial.compressedData.z; }