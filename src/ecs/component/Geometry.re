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
  // let texCoords = [|0.5, 1.0, 0.0, 0.0, 1.0, 0.0|];

  // [|
  //   0.0,
  //   1.0,
  //   0.0,
  //   0.0,
  //   0.0,
  //   1.0,
  //   (-1.0),
  //   (-1.0),
  //   0.0,
  //   0.0,
  //   0.0,
  //   1.0,
  //   1.0,
  //   (-1.0),
  //   0.0,
  //   0.0,
  //   0.0,
  //   1.0,
  // |];

  (vertices, normals);
};

let buildTriangleIndexData = () => {
  let indices = [|0, 1, 2|];

  indices;
};

let buildPlaneVertexData = () => {
  // [|
  //   1.0,
  //   0.0,
  //   (-1.0),
  //   0.0,
  //   1.0,
  //   0.0,
  //   1.0,
  //   0.0,
  //   1.0,
  //   0.0,
  //   1.0,
  //   0.0,
  //   (-1.0),
  //   0.0,
  //   1.0,
  //   0.0,
  //   1.0,
  //   0.0,
  //   (-1.0),
  //   0.0,
  //   (-1.0),
  //   0.0,
  //   1.0,
  //   0.0,
  // |];

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
  // let texCoords = [|0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0|];

  (vertices, normals);
};

let buildPlaneIndexData = () => {
  let indices = [|2, 1, 0, 0, 3, 2|];

  indices;
};

let computeVertexCount = vertices => {
  (vertices |> Js.Array.length) / 3;
};

let computeIndexCount = indices => {
  indices |> Js.Array.length;
};

let unsafeGetVertexData = (geometry, state) => {
  state.geometry.vertexDataMap |> ImmutableSparseMap.unsafeGet(geometry);
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

let unsafeGetIndexData = (geometry, state) => {
  state.geometry.indexDataMap |> ImmutableSparseMap.unsafeGet(geometry);
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
