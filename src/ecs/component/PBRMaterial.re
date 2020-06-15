open StateType;

let create = state => {
  (
    state.pbrMaterial.index,
    {
      ...state,
      pbrMaterial: {
        ...state.pbrMaterial,
        index: state.pbrMaterial.index |> succ,
      },
    },
  );
};

// let setAmbient = (pbrMaterial, ambient, state) => {
//   ...state,
//   pbrMaterial: {
//     ...state.pbrMaterial,
//     ambientMap:
//       state.pbrMaterial.ambientMap
//       |> ImmutableSparseMap.set(pbrMaterial, ambient),
//   },
// };

let unsafeGetDiffuse = (pbrMaterial, state) => {
  state.pbrMaterial.diffuseMap
  |> ImmutableSparseMap.unsafeGet(pbrMaterial);
};

let setDiffuse = (pbrMaterial, diffuse, state) => {
  ...state,
  pbrMaterial: {
    ...state.pbrMaterial,
    diffuseMap:
      state.pbrMaterial.diffuseMap
      |> ImmutableSparseMap.set(pbrMaterial, diffuse),
  },
};

let unsafeGetSpecular = (pbrMaterial, state) => {
  state.pbrMaterial.specularMap
  |> ImmutableSparseMap.unsafeGet(pbrMaterial);
};

let setSpecular = (pbrMaterial, specular, state) => {
  ...state,
  pbrMaterial: {
    ...state.pbrMaterial,
    specularMap:
      state.pbrMaterial.specularMap
      |> ImmutableSparseMap.set(pbrMaterial, specular),
  },
};

let unsafeGetMetalness = (pbrMaterial, state) => {
  state.pbrMaterial.metalnessMap
  |> ImmutableSparseMap.unsafeGet(pbrMaterial);
};

let setMetalness = (pbrMaterial, metalness, state) => {
  ...state,
  pbrMaterial: {
    ...state.pbrMaterial,
    metalnessMap:
      state.pbrMaterial.metalnessMap
      |> ImmutableSparseMap.set(pbrMaterial, metalness),
  },
};



let unsafeGetRoughness = (pbrMaterial, state) => {
  state.pbrMaterial.roughnessMap
  |> ImmutableSparseMap.unsafeGet(pbrMaterial);
};

let setRoughness = (pbrMaterial, roughness, state) => {
  ...state,
  pbrMaterial: {
    ...state.pbrMaterial,
    roughnessMap:
      state.pbrMaterial.roughnessMap
      |> ImmutableSparseMap.set(pbrMaterial, roughness),
  },
};

// let setIllum = (pbrMaterial, illum, state) => {
//   ...state,
//   pbrMaterial: {
//     ...state.pbrMaterial,
//     illumMap:
//       state.pbrMaterial.illumMap
//       |> ImmutableSparseMap.set(pbrMaterial, illum),
//   },
// };

// let setDissolve = (pbrMaterial, dissolve, state) => {
//   ...state,
//   pbrMaterial: {
//     ...state.pbrMaterial,
//     dissolveMap:
//       state.pbrMaterial.dissolveMap
//       |> ImmutableSparseMap.set(pbrMaterial, dissolve),
//   },
// };

let getCount = state => {
  state.pbrMaterial.index;
};

let getAllPBRMaterials = state => {
  ArrayUtils.range(0, state.pbrMaterial.index);
};
