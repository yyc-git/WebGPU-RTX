open StateType;

let create = state => {
  (
    state.gameObject.index,
    {
      ...state,
      gameObject: {
        ...state.gameObject,
        index: state.gameObject.index |> succ,
      },
    },
  );
};

let addTransform = (gameObject, transform, state) => {
  ...state,
  gameObject: {
    ...state.gameObject,
    transformMap:
      state.gameObject.transformMap
      |> ImmutableSparseMap.set(gameObject, transform),
  },
};

let addGeometry = (gameObject, geometry, state) => {
  ...state,
  gameObject: {
    ...state.gameObject,
    geometryMap:
      state.gameObject.geometryMap
      |> ImmutableSparseMap.set(gameObject, geometry),
  },
};

let addPBRMaterial = (gameObject, material, state) => {
  ...state,
  gameObject: {
    ...state.gameObject,
    pbrMaterialMap:
      state.gameObject.pbrMaterialMap
      |> ImmutableSparseMap.set(gameObject, material),
  },
};

let addShader = (gameObject, shader, state) => {
  ...state,
  gameObject: {
    ...state.gameObject,
    shaderMap:
      state.gameObject.shaderMap |> ImmutableSparseMap.set(gameObject, shader),
  },
};

let addDirectionLight = (gameObject, light, state) => {
  ...state,
  gameObject: {
    ...state.gameObject,
    directionLightMap:
      state.gameObject.directionLightMap
      |> ImmutableSparseMap.set(gameObject, light),
  },
};

let addTransformAnimation = (gameObject, animation, state) => {
  ...state,
  gameObject: {
    ...state.gameObject,
    transformAnimationMap:
      state.gameObject.transformAnimationMap
      |> ImmutableSparseMap.set(gameObject, animation),
  },
};

let addCameraView = (gameObject, cameraView, state) => {
  ...state,
  gameObject: {
    ...state.gameObject,
    cameraViewMap:
      state.gameObject.cameraViewMap
      |> ImmutableSparseMap.set(gameObject, cameraView),
  },
};

let addArcballCameraController = (gameObject, cameraController, state) => {
  ...state,
  gameObject: {
    ...state.gameObject,
    arcballCameraControllerMap:
      state.gameObject.arcballCameraControllerMap
      |> ImmutableSparseMap.set(gameObject, cameraController),
  },
};

let unsafeGetTransform = (gameObject, state) => {
  state.gameObject.transformMap |> ImmutableSparseMap.unsafeGet(gameObject);
};

let unsafeGetPBRMaterial = (gameObject, state) => {
  state.gameObject.pbrMaterialMap
  |> ImmutableSparseMap.unsafeGet(gameObject);
};

let unsafeGetShader = (gameObject, state) => {
  state.gameObject.shaderMap |> ImmutableSparseMap.unsafeGet(gameObject);
};

let unsafeGetGeometry = (gameObject, state) => {
  state.gameObject.geometryMap |> ImmutableSparseMap.unsafeGet(gameObject);
};

let getAllGeometryGameObjects = state => {
  state.gameObject.geometryMap |> ImmutableSparseMap.getValidKeys;
};
