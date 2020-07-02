
vec3 shootIndirectRay(accelerationStructureEXT topLevelAS, vec3 origin,
                      vec3 rayDir, float tMin) {
  float tMax = 1.0e38f; // The farthest distance we'll count as a hit
  uint flags = gl_RayFlagsNoneEXT;

  prd.hitValue = vec3(0.0);

  traceRayEXT(topLevelAS, // acceleration structure
          flags,      // rayFlags
          0xFF,       // cullMask
          0,          // sbtRecordOffset
          0,          // sbtRecordStride
          0,          // missIndex
          origin,     // ray origin
          tMin,       // ray min range
          rayDir,     // ray direction
          tMax,       // ray max range
          0           // payload (location = 0)
  );

  return prd.hitValue;
}