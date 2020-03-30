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
        viewMatrixInverseMap:
          state.cameraView.viewMatrixInverseMap
          |> ImmutableSparseMap.set(
               newCameraView,
               Matrix4.createIdentityMatrix4(),
             ),
        lastViewMatrixInverseMap:
          state.cameraView.lastViewMatrixInverseMap
          |> ImmutableSparseMap.set(newCameraView, None),
        projectionMatrixInverseMap:
          state.cameraView.viewMatrixInverseMap
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

let unsafeGetViewMatrixInverse = (cameraView, state) => {
  state.cameraView.viewMatrixInverseMap
  |> ImmutableSparseMap.unsafeGet(cameraView);
};

let setViewMatrixInverse = (cameraView, mat, state) => {
  ...state,
  cameraView: {
    ...state.cameraView,
    viewMatrixInverseMap:
      state.cameraView.viewMatrixInverseMap
      |> ImmutableSparseMap.set(cameraView, mat),
  },
};

let unsafeGetLastViewMatrixInverse = (cameraView, state) => {
  state.cameraView.lastViewMatrixInverseMap
  |> ImmutableSparseMap.unsafeGet(cameraView);
};

let setLastViewMatrixInverse = (cameraView, mat, state) => {
  ...state,
  cameraView: {
    ...state.cameraView,
    lastViewMatrixInverseMap:
      state.cameraView.lastViewMatrixInverseMap
      |> ImmutableSparseMap.set(cameraView, mat),
  },
};

let unsafeGetProjectionMatrixInverse = (cameraView, state) => {
  state.cameraView.projectionMatrixInverseMap
  |> ImmutableSparseMap.unsafeGet(cameraView);
};

let setProjectionMatrixInverse = (cameraView, mat, state) => {
  ...state,
  cameraView: {
    ...state.cameraView,
    projectionMatrixInverseMap:
      state.cameraView.projectionMatrixInverseMap
      |> ImmutableSparseMap.set(cameraView, mat),
  },
};

let update = ((aspect, fovy, near, far), (lookFrom, lookAt, up), state) => {
  let currentCameraView = state |> unsafeGetCurrentCameraView;

  state
  |> setLastViewMatrixInverse(
       currentCameraView,
       unsafeGetViewMatrixInverse(currentCameraView, state)->Some,
     )
  |> setProjectionMatrixInverse(
       currentCameraView,
       Matrix4.createIdentityMatrix4()
       |> Matrix4.buildPerspective((fovy, aspect, near, far))
       |> Matrix4.invert(_, Matrix4.createIdentityMatrix4()),
     )
  |> setViewMatrixInverse(
       currentCameraView,
       Matrix4.createIdentityMatrix4()
       |> Matrix4.setLookAt(lookFrom, lookAt, up)
       |> Matrix4.invert(_, Matrix4.createIdentityMatrix4()),
     );
};
