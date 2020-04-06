open StateType;

let create = state => {
  (
    state.phongMaterial.index,
    {
      ...state,
      phongMaterial: {
        ...state.phongMaterial,
        index: state.phongMaterial.index |> succ,
      },
    },
  );
};

// let setAmbient = (phongMaterial, ambient, state) => {
//   ...state,
//   phongMaterial: {
//     ...state.phongMaterial,
//     ambientMap:
//       state.phongMaterial.ambientMap
//       |> ImmutableSparseMap.set(phongMaterial, ambient),
//   },
// };

let unsafeGetDiffuse = (phongMaterial, state) => {
  state.phongMaterial.diffuseMap
  |> ImmutableSparseMap.unsafeGet(phongMaterial);
};

let setDiffuse = (phongMaterial, diffuse, state) => {
  ...state,
  phongMaterial: {
    ...state.phongMaterial,
    diffuseMap:
      state.phongMaterial.diffuseMap
      |> ImmutableSparseMap.set(phongMaterial, diffuse),
  },
};

let unsafeGetSpecular = (phongMaterial, state) => {
  state.phongMaterial.specularMap
  |> ImmutableSparseMap.unsafeGet(phongMaterial);
};

let setSpecular = (phongMaterial, specular, state) => {
  ...state,
  phongMaterial: {
    ...state.phongMaterial,
    specularMap:
      state.phongMaterial.specularMap
      |> ImmutableSparseMap.set(phongMaterial, specular),
  },
};

let unsafeGetShininess = (phongMaterial, state) => {
  state.phongMaterial.shininessMap
  |> ImmutableSparseMap.unsafeGet(phongMaterial);
};

let setShininess = (phongMaterial, shininess, state) => {
  ...state,
  phongMaterial: {
    ...state.phongMaterial,
    shininessMap:
      state.phongMaterial.shininessMap
      |> ImmutableSparseMap.set(phongMaterial, shininess),
  },
};

// let setIllum = (phongMaterial, illum, state) => {
//   ...state,
//   phongMaterial: {
//     ...state.phongMaterial,
//     illumMap:
//       state.phongMaterial.illumMap
//       |> ImmutableSparseMap.set(phongMaterial, illum),
//   },
// };

// let setDissolve = (phongMaterial, dissolve, state) => {
//   ...state,
//   phongMaterial: {
//     ...state.phongMaterial,
//     dissolveMap:
//       state.phongMaterial.dissolveMap
//       |> ImmutableSparseMap.set(phongMaterial, dissolve),
//   },
// };
