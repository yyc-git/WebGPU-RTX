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

let unsafeGetHitGroupIndex = (shader, state) => {
  state.shader.hitGroupIndexMap |> ImmutableSparseMap.unsafeGet(shader);
};

let setHitGroupIndex = (shader, index, state) => {
  ...state,
  shader: {
    ...state.shader,
    hitGroupIndexMap:
      state.shader.hitGroupIndexMap |> ImmutableSparseMap.set(shader, index),
  },
};
