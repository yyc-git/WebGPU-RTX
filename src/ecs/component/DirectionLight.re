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

let setIntensity = (directionLight, intensity, state) => {
  ...state,
  directionLight: {
    ...state.directionLight,
    intensityMap:
      state.directionLight.intensityMap
      |> ImmutableSparseMap.set(directionLight, intensity),
  },
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
