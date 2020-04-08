// #version 450
// #pragma shader_stage(fragment)

// layout(location = 0) in vec2 uv;
// layout(location = 0) out vec4 outColor;

// layout(std140, set = 0, binding = 0) buffer PixelBuffer { vec4 pixels[]; }
// pixelBuffer;

// layout(set = 0, binding = 1) uniform ScreenDimension { vec2 resolution; };

// void main() {
//   const ivec2 bufferCoord = ivec2(floor(uv * resolution));
//   const vec2 fragCoord = (uv * resolution);
//   const uint pixelIndex = bufferCoord.y * uint(resolution.x) + bufferCoord.x;

//   vec4 pixelColor = pixelBuffer.pixels[pixelIndex];
//   outColor = pixelColor;
// }

// TODO remove test code

#version 450
#pragma shader_stage(fragment)

#include "../../shaders/definition.glsl"

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D gPositionTexture;
layout(binding = 1) uniform sampler2D gNormalTexture;
layout(binding = 2) uniform sampler2D gDiffuseTexture;
// layout(binding = 3) uniform sampler2D gSpecularTexture;
// layout(binding = 4) uniform sampler2D gShininessTexture;
layout(binding = 3) uniform sampler2D gDepthShininessTexture;
// layout(binding = 5) uniform sampler2D gDepthTexture;

// float LinearDepth(float depth) {
//   return (depth * NEAR_Z) / (FAR_Z - depth * (FAR_Z - NEAR_Z));
// }

float LinearDepth(float depth) 
{
    // float z = depth * 2.0 - 1.0; // back to NDC 

    // // return (2.0 * near * far) / (far + near - z * (far - near));	
    // return (2.0 * NEAR_Z) / (FAR_Z + NEAR_Z - z * (FAR_Z - NEAR_Z));	

    // float z = depth * 2.0 - 1.0; // back to NDC 

    // return (2.0 * near * far) / (far + near - z * (far - near));	
    return (2.0 * NEAR_Z) / (FAR_Z + NEAR_Z - depth * (FAR_Z - NEAR_Z));	
}

void main() {
  vec2 lanuchIndex = uv;

  vec4 worldPosition = texture(gPositionTexture, lanuchIndex);
  vec3 worldNormal = texture(gNormalTexture, lanuchIndex).rgb;
  vec3 diffuse = texture(gDiffuseTexture, lanuchIndex).rgb;
  // vec3 specular = texture(gSpecularTexture, lanuchIndex).rgb;
  vec4 depthShininess =  texture(gDepthShininessTexture, lanuchIndex);
  float depth = depthShininess.r;
  float shininess = depthShininess.g;


    // outColor = vec4(clamp(vec3(worldPosition) * 100, 0.0, 1.0), 1.0);
    // outColor = vec4(vec3(worldPosition), 1.0);
    // outColor = vec4(diffuse, 1.0);
    // outColor = vec4(depthShininess.r, depthShininess.g/ 72.0, depthShininess.ba);
    // outColor = vec4(worldNormal, 1.0);

  if (worldPosition.w == 0.0) {
    // outColor = vec4(diffuse, 1.0);
    // outColor = vec4(1.0,0.0,0.0,1.0);


    // outColor = vec4(LinearDepth(depthShininess.r), 0.0,0.0,0.0);
    // outColor = vec4(vec3(worldPosition), 1.0);
    outColor = vec4(vec3(worldNormal), 1.0);
  } else {
    outColor = vec4(0.5);
  }
    // outColor = vec4(0.5);
}

