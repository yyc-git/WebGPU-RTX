layout(std140, set = 1, binding = 0) uniform PhongMaterial {
  //   vec4 ambient;
  vec4 diffuse;
  // vec4 specular;
  // include shininess
  vec4 compressedData;
}
uPhongMaterial;

vec3 getMaterialDiffuse() { return vec3(uPhongMaterial.diffuse); }

// vec3 getMaterialSpecular() { return vec3(uPhongMaterial.specular); }

float getMaterialShininess() { return uPhongMaterial.compressedData.x; }