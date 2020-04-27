vec2 getJitterdUV(vec2 uv, vec2 jitter) { return uv; }

vec2 getUnJitterdUV(vec2 uv, vec2 jitter) {
  return uv - vec2(jitter.x, jitter.y);
}