vec2 getUnjitterdUV(vec2 uv, vec2 jitter) {
  return uv + vec2(-jitter.x / 2.0, -jitter.y / 2.0);
}

vec2 getJitterdUV(vec2 uv) { return uv; }