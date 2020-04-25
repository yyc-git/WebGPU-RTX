open StateType;

let create = state => {
  (
    state.directionLight.index,
    {
      ...state,
      directionLight: {
        ...state.directionLight,
        index: state.directionLight.index |> succ,
      },
    },
  );
};

let unsafeGetIntensity = (directionLight, state) => {
  state.directionLight.intensityMap
  |> ImmutableSparseMap.unsafeGet(directionLight);
};

let setIntensity = (directionLight, intensity, state) => {
  ...state,
  directionLight: {
    ...state.directionLight,
    intensityMap:
      state.directionLight.intensityMap
      |> ImmutableSparseMap.set(directionLight, intensity),
  },
};

let unsafeGetPosition = (directionLight, state) => {
  state.directionLight.positionMap
  |> ImmutableSparseMap.unsafeGet(directionLight);
};

let setPosition = (directionLight, position, state) => {
  ...state,
  directionLight: {
    ...state.directionLight,
    positionMap:
      state.directionLight.positionMap
      |> ImmutableSparseMap.set(directionLight, position),
  },
};

let getAllLights = state => {
  ArrayUtils.range(0, state.directionLight.index);
};

let getLightCount = state => {
  getAllLights(state) |> Js.Array.length;
};
