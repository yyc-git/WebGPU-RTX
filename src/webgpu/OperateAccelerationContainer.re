open StateType;

let _getData = state => state.rayTracing.accelerationContainer;

let unsafeGetData = state => {
  let {
    geometryContainers,
    instanceContainer,
    instanceBufferArrayBuffer,
    instanceBuffer,
  } =
    _getData(state);

  (
    geometryContainers |> Js.Option.getExn,
    instanceContainer |> Js.Option.getExn,
    instanceBufferArrayBuffer |> Js.Option.getExn,
    instanceBuffer |> Js.Option.getExn,
  );
};

let setData =
    (
      geometryContainers,
      instanceContainer,
      instanceBufferArrayBuffer,
      instanceBuffer,
      state,
    ) => {
  ...state,
  rayTracing: {
    ...state.rayTracing,
    accelerationContainer: {
      geometryContainers: Some(geometryContainers),
      instanceBufferArrayBuffer: Some(instanceBufferArrayBuffer),
      instanceBuffer: Some(instanceBuffer),
      instanceContainer: Some(instanceContainer),
    },
  },
};
