open StateType;

let _getData = state => state.rayTracing.accelerationContainer;

let unsafeGetData = state => {
  let {geometryContainers, instanceContainer} = _getData(state);

  (
    geometryContainers |> Js.Option.getExn,
    instanceContainer |> Js.Option.getExn,
  );
};

let setData = (geometryContainers, instanceContainer, state) => {
  ...state,
  rayTracing: {
    ...state.rayTracing,
    accelerationContainer: {
      geometryContainers: Some(geometryContainers),
      instanceContainer: Some(instanceContainer),
    },
  },
};
