open StateType;

let create = state => {
  (
    state.transformAnimation.index,
    {
      ...state,
      transformAnimation: {
        ...state.transformAnimation,
        index: state.transformAnimation.index |> succ,
      },
    },
  );
};

let setDynamicTransform = (transformAnimation, transform, state) => {
  ...state,
  transformAnimation: {
    ...state.transformAnimation,
    dynamicTransformMap:
      state.transformAnimation.dynamicTransformMap
      |> ImmutableSparseMap.set(transformAnimation, transform),
  },
};
