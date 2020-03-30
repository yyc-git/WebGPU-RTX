open StateType;

let create = state => {
  (
    state.shader.index,
    {
      ...state,
      shader: {
        ...state.shader,
        index: state.shader.index |> succ,
      },
    },
  );
};

let setHitGroupIndex = (shader, index, state) => {
  ...state,
  shader: {
    ...state.shader,
    hitGroupIndexMap:
      state.shader.hitGroupIndexMap
      |> ImmutableSparseMap.set(shader, index),
  },
};
