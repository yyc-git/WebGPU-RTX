
struct InstanceData {
  // TODO perf: remove compressedData by pack to normalMatrix

  /*
   because scalar not work(not support float objId; mat4 modelMatrix;),
   so all aligned to vec4
   */

  // include vertexIndex, materialIndex
  vec4 compressedData;

  mat3 normalMatrix;
  mat4 modelMatrix;
};

struct Vertex {
  vec4 position;
  vec4 normal;
  // TODO perf: pack texCoord to position,normal
  //   vec4 texCoord;
};

/*
extract this to avoid duplicate instead of move this into InstanceData.
*/
struct GeometryOffsetData {
  uint vertexOffset;
  uint indexOffset;
};

struct PBRMaterial {
  vec4 diffuse;

  // include metalness, roughness, specular
  vec4 compressedData;
};

hitAttributeEXT vec3 attribs;

layout(std140, set = 1, binding = 4) buffer SceneDesc { InstanceData i[]; }
sceneDesc;

layout(scalar, set = 1, binding = 5) buffer SceneGeometryOffsetData {
  GeometryOffsetData o[];
}
sceneGeometryOffsetData;

// TODO use array of blocks!how to upload data???
layout(scalar, set = 1, binding = 6) buffer Vertices { Vertex v[]; }
vertices;
layout(scalar, set = 1, binding = 7) buffer Indices { uint i[]; }
indices;

layout(std140, set = 1, binding = 8) buffer MatColorBufferObject {
  PBRMaterial m[];
}
materials;

vec4 _getInstanceDataCompressedData(InstanceData instanceData) {
  return instanceData.compressedData;
}

uint _getGeometryIndex(vec4 compressedData) { return uint(compressedData.x); }

uint _getMaterialIndex(vec4 compressedData) { return uint(compressedData.y); }

uint _getVertexOffset(GeometryOffsetData geometryOffsetData) {
  return geometryOffsetData.vertexOffset;
}

uint _getIndexOffset(GeometryOffsetData geometryOffsetData) {
  return geometryOffsetData.indexOffset;
}

mat3 _getNormalMatrix(InstanceData instanceData) {
  return instanceData.normalMatrix;
}

mat4 _getModelMatrix(InstanceData instanceData) {
  return instanceData.modelMatrix;
}

InstanceData _getInstanceData(uint instanceIndex) {
  return sceneDesc.i[instanceIndex];
}

GeometryOffsetData _getGeometryOffsetData(uint geometryIndex) {
  return sceneGeometryOffsetData.o[geometryIndex];
}

PBRMaterial _getMaterial(uint materialIndex) {
  return materials.m[materialIndex];
}

vec3 _getMaterialDiffuse(PBRMaterial mat) { return vec3(mat.diffuse); }

float _getMaterialMetalness(PBRMaterial mat) { return mat.compressedData.x; }

float _getMaterialRoughness(PBRMaterial mat) { return mat.compressedData.y; }

float _getMaterialSpecular(PBRMaterial mat) { return mat.compressedData.z; }

ivec3 _getTriangleIndices(uint indexOffset, uint primitiveIndex) {
  return ivec3(indices.i[indexOffset + 3 * primitiveIndex + 0],
               indices.i[indexOffset + 3 * primitiveIndex + 1],
               indices.i[indexOffset + 3 * primitiveIndex + 2]);
}

Vertex _getTriangleVertex(uint vertexOffset, uint index) {
  return vertices.v[vertexOffset + index];
}

struct HitShadingData {
  vec3 worldPosition;
  vec3 worldNormal;
  vec3 V;
  vec3 materialDiffuse;
  // vec3 materialSpecularColor;
  float materialMetalness;
  float materialRoughness;
  float materialSpecular;
};

HitShadingData getHitShadingData(uint instanceIndex, uint primitiveIndex) {
  InstanceData instanceData = _getInstanceData(instanceIndex);

  vec4 instanceDataCompressedData =
      _getInstanceDataCompressedData(instanceData);
  uint geometryIndex = _getGeometryIndex(instanceDataCompressedData);
  uint materialIndex = _getMaterialIndex(instanceDataCompressedData);
  GeometryOffsetData geometryOffsetData = _getGeometryOffsetData(geometryIndex);
  uint vertexOffset = _getVertexOffset(geometryOffsetData);
  uint indexOffset = _getIndexOffset(geometryOffsetData);

  // Indices of the triangle
  ivec3 ind = _getTriangleIndices(indexOffset, primitiveIndex);

  // Vertex of the triangle
  Vertex v0 = _getTriangleVertex(vertexOffset, ind.x);
  Vertex v1 = _getTriangleVertex(vertexOffset, ind.y);
  Vertex v2 = _getTriangleVertex(vertexOffset, ind.z);

  const vec3 barycentrics =
      vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  // Computing the normal at hit position
  vec3 localNormal = vec3(v0.normal) * barycentrics.x +
                     vec3(v1.normal) * barycentrics.y +
                     vec3(v2.normal) * barycentrics.z;

  // Computing the coordinates of the hit position
  vec3 localPos = vec3(v0.position) * barycentrics.x +
                  vec3(v1.position) * barycentrics.y +
                  vec3(v2.position) * barycentrics.z;

  PBRMaterial mat = _getMaterial(materialIndex);

  HitShadingData data;
  data.worldPosition =
      vec3(_getModelMatrix(instanceData) * vec4(localPos, 1.0));
  data.worldNormal = normalize(_getNormalMatrix(instanceData) * localNormal);
  data.V = normalize(uCamera.cameraPosition.xyz - data.worldPosition);
  data.materialDiffuse = _getMaterialDiffuse(mat);
  data.materialMetalness = _getMaterialMetalness(mat);
  data.materialRoughness = _getMaterialRoughness(mat);
  data.materialSpecular = _getMaterialSpecular(mat);

  return data;
}
