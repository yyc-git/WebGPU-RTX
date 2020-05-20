uint getFrame(vec4 compressedData) { return uint(compressedData.x); }

uint getLightCount(vec4 compressedData) { return uint(compressedData.y); }

// bool getDoIndirectGI(vec4 compressedData) { return compressedData.z > 0.0; }
