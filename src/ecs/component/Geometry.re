open StateType;

let create = state => {
  (
    state.geometry.index,
    {
      ...state,
      geometry: {
        ...state.geometry,
        index: state.geometry.index |> succ,
      },
    },
  );
};

let buildTriangleVertexData = () => {
  let vertices = [|0.0, 1.0, 0.0, (-1.0), (-1.0), 0.0, 1.0, (-1.0), 0.0|];
  let normals = [|0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0|];
  let texCoords = [|0.5, 1.0, 0.0, 0.0, 1.0, 0.0|];

  (vertices, normals, texCoords);
};

let buildTriangleIndexData = () => {
  let indices = [|0, 1, 2|];

  indices;
};

let buildPlaneVertexData = () => {
  let vertices = [|
    1.0,
    0.0,
    (-1.0),
    1.0,
    0.0,
    1.0,
    (-1.0),
    0.0,
    1.0,
    (-1.0),
    0.0,
    (-1.0),
  |];
  let normals = [|
    0.0,
    1.0,
    0.0,
    0.0,
    1.0,
    0.0,
    0.0,
    1.0,
    0.0,
    0.0,
    1.0,
    0.0,
  |];
  let texCoords = [|0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0|];

  (vertices, normals, texCoords);
};

let buildPlaneIndexData = () => {
  let indices = [|2, 1, 0, 0, 3, 2|];

  indices;
};

let setVertexData = (geometry, vertexData, state) => {
  ...state,
  geometry: {
    ...state.geometry,
    vertexDataMap:
      state.geometry.vertexDataMap
      |> ImmutableSparseMap.set(geometry, vertexData),
  },
};

let setIndexData = (geometry, indexData, state) => {
  ...state,
  geometry: {
    ...state.geometry,
    indexDataMap:
      state.geometry.indexDataMap
      |> ImmutableSparseMap.set(geometry, indexData),
  },
};

let getAllVertexData = state => {
  state.geometry.vertexDataMap |> ImmutableSparseMap.getValidValues;
};

let getAllIndexData = state => {
  state.geometry.indexDataMap |> ImmutableSparseMap.getValidValues;
};



let getCount = state => {
  state.geometry.index;
};