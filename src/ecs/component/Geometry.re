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

let buildSphereVertexData = (radius, bands) => {
  let latitudeBands = bands;
  let longitudeBands = bands;

  let vertices = [||];
  let normals = [||];
  // let texCoords = [||];
  // let indices = [||];

  for (latNumber in 0 to latitudeBands) {
    let latNumber = latNumber |> float_of_int;
    let latitudeBands = latitudeBands |> float_of_int;

    let theta = latNumber *. Js.Math._PI /. latitudeBands;
    let sinTheta = Js.Math.sin(theta);
    let cosTheta = Js.Math.cos(theta);

    for (longNumber in 0 to longitudeBands) {
      let longNumber = longNumber |> float_of_int;
      let longitudeBands = longitudeBands |> float_of_int;

      let phi = longNumber *. 2. *. Js.Math._PI /. longitudeBands;
      let sinPhi = Js.Math.sin(phi);
      let cosPhi = Js.Math.cos(phi);

      let x = radius *. cosPhi *. sinTheta;
      let y = radius *. cosTheta;
      let z = radius *. sinPhi *. sinTheta;
      let u = 1. -. longNumber /. longitudeBands;
      let v = 1. -. latNumber /. latitudeBands;

      vertices
      |> ArrayUtils.push(x)
      |> ArrayUtils.push(y)
      |> ArrayUtils.push(z)
      |> ignore;

      normals
      |> ArrayUtils.push(x)
      |> ArrayUtils.push(y)
      |> ArrayUtils.push(z)
      |> ignore;
      // texCoords |> ArrayUtils.push(u) |> ArrayUtils.push(v) |> ignore;
    };
  };

  (vertices, normals);
};

let buildSphereIndexData = (bands) => {
  let latitudeBands = bands;
  let longitudeBands = bands;

  let indices = [||];

  for (latNumber in 0 to latitudeBands - 1) {
    for (longNumber in 0 to longitudeBands - 1) {
      let first = latNumber * (longitudeBands + 1) + longNumber;
      let second = first + longitudeBands + 1;

      indices
      |> ArrayUtils.push(first + 1)
      |> ArrayUtils.push(second)
      |> ArrayUtils.push(first)
      |> ArrayUtils.push(first + 1)
      |> ArrayUtils.push(second + 1)
      |> ArrayUtils.push(second)
      |> ignore;
    };
  };

  indices;
};

let computeVerticesCount = vertices => {
  (vertices |> Js.Array.length) / 3;
};

let computeNormalsCount = normals => {
  (normals |> Js.Array.length) / 3;
};

let computeVertexCount = vertices => {
  computeVerticesCount(vertices);
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

let getAllGeometries = state => {
  ArrayUtils.range(0, state.geometry.index);
};
