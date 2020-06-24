
// Utility function to get a vector perpendicular to an input vector
//    (from "Efficient Construction of Perpendicular Vectors Without Branching")
vec3 getPerpendicularVector(vec3 u) {
  vec3 a = abs(u);
  uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
  uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
  uint zm = 1 ^ (xm | ym);
  return cross(u, vec3(xm, ym, zm));
}

// // Takes our seed, updates it, and returns a pseudorandom float in [0..1]
// float _nextRand(inout uint s) {
//   s = (1664525u * s + 1013904223u);
//   return float(s & 0x00FFFFFF) / float(0x01000000);
// }

vec3 getCosHemisphereSample(float r1, float r2, vec3 hitNorm) {
  // Get 2 random numbers to select our sample with
  vec2 randVal = vec2(r1, r2);

  // Cosine weighted hemisphere sample from RNG
  vec3 bitangent = getPerpendicularVector(hitNorm);
  vec3 tangent = cross(bitangent, hitNorm);
  float r = sqrt(randVal.x);
  float phi = 2.0f * 3.14159265f * randVal.y;

  // Get our cosine-weighted hemisphere lobe sample direction
  return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) +
         hitNorm.xyz * sqrt(1 - randVal.x);
}
