open StateType;

let create = state => {
  let newCameraView = state.cameraView.index;

  (
    newCameraView,
    {
      ...state,
      cameraView: {
        ...state.cameraView,
        index: state.cameraView.index |> succ,
        viewMatrixMap:
          state.cameraView.viewMatrixMap
          |> ImmutableSparseMap.set(
               newCameraView,
               Matrix4.createIdentityMatrix4(),
             ),
        projectionMatrixMap:
          state.cameraView.projectionMatrixMap
          |> ImmutableSparseMap.set(
               newCameraView,
               Matrix4.createIdentityMatrix4(),
             ),
      },
    },
  );
};

let unsafeGetCurrentCameraView = state => {
  state.cameraView.currentCameraView |> Js.Option.getExn;
};

let setCurrentCameraView = (cameraView, state) => {
  ...state,
  cameraView: {
    ...state.cameraView,
    currentCameraView: Some(cameraView),
  },
};

let unsafeGetCameraPosition = (cameraView, state) => {
  state.cameraView.cameraPositionMap
  |> ImmutableSparseMap.unsafeGet(cameraView);
};

let setCameraPosition = (cameraView, cameraPosition, state) => {
  ...state,
  cameraView: {
    ...state.cameraView,
    cameraPositionMap:
      state.cameraView.cameraPositionMap
      |> ImmutableSparseMap.set(cameraView, cameraPosition),
  },
};

let unsafeGetViewMatrix = (cameraView, state) => {
  state.cameraView.viewMatrixMap |> ImmutableSparseMap.unsafeGet(cameraView);
};

let setViewMatrix = (cameraView, mat, state) => {
  ...state,
  cameraView: {
    ...state.cameraView,
    viewMatrixMap:
      state.cameraView.viewMatrixMap
      |> ImmutableSparseMap.set(cameraView, mat),
  },
};

let unsafeGetProjectionMatrix = (cameraView, state) => {
  state.cameraView.projectionMatrixMap
  |> ImmutableSparseMap.unsafeGet(cameraView);
};

let setProjectionMatrix = (cameraView, mat, state) => {
  ...state,
  cameraView: {
    ...state.cameraView,
    projectionMatrixMap:
      state.cameraView.projectionMatrixMap
      |> ImmutableSparseMap.set(cameraView, mat),
  },
};

let update = ((aspect, fovy, near, far), (lookFrom, lookAt, up), state) => {
  let currentCameraView = state |> unsafeGetCurrentCameraView;

  state
  |> setCameraPosition(currentCameraView, lookFrom)
  |> setProjectionMatrix(
       currentCameraView,
       Matrix4.createIdentityMatrix4()
       |> Matrix4.buildPerspective((fovy, aspect, near, far)),
     )
  |> setViewMatrix(
       currentCameraView,
       Matrix4.createIdentityMatrix4()
       |> Matrix4.setLookAt(lookFrom, lookAt, up),
     );
};
